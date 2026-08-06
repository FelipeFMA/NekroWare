#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <winhttp.h>
#include <commctrl.h>
#include <shellapi.h>
#include <string>
#include <map>
#include <cstdint>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <iostream>
#include "offsets.h"
#include "../Memory/MemoryManager.h"

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")
// Task dialogs need comctl32 v6.
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Maps "Namespace::Member" -> address of the runtime variable in offsets.h.
// The list is generated from the static dump (tools\gen_offsets_registry.ps1)
// so the compile-time values remain as the offline fallback.
#define OFFSET_REG(ns, member) { #ns "::" #member, &Offsets::ns::member },
inline std::map<std::string, uintptr_t*> g_OffsetRegistry = {
#include "offsets_registry.h"
};
#undef OFFSET_REG

namespace OffsetsDynamic
{
	// Extracts the "version-xxxxxxxxxxxxxxxx" folder name from the running
	// RobloxPlayerBeta.exe path. Only Fishstrap installs are supported: the
	// path must contain the Fishstrap launcher folder
	// (...\Fishstrap\Versions\version-xxx\RobloxPlayerBeta.exe). Returns ""
	// when the client is not running from Fishstrap.
	inline std::string GetRunningRobloxVersion()
	{
		HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, Memory->getProcessId());
		if (snap == INVALID_HANDLE_VALUE)
			return "";

		MODULEENTRY32 me{};
		me.dwSize = sizeof(me);
		bool found = false;
		if (Module32First(snap, &me))
		{
			do
			{
				if (_stricmp(me.szModule, "RobloxPlayerBeta.exe") == 0)
				{
					found = true;
					break;
				}
			} while (Module32Next(snap, &me));
		}
		CloseHandle(snap);
		if (!found)
			return "";

		std::string lower = me.szExePath;
		for (auto& c : lower)
			c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
		if (lower.find("fishstrap") == std::string::npos)
			return "";

		size_t start = lower.find("version-");
		if (start == std::string::npos)
			return "";

		size_t end = lower.find_first_of("\\/", start);
		return lower.substr(start, end - start);
	}

	// MessageBox telling the user that only Fishstrap is supported, with an
	// "OK" (close NekroWare) and a "Download Fishstrap" (opens fishstrap.app)
	// button. Returns true if the user chose to open the download page.
	inline bool ConfirmFishstrapRequired()
	{
		TASKDIALOG_BUTTON buttons[] = {
			{ 1, L"OK" },
			{ 2, L"Download Fishstrap" },
		};
		TASKDIALOGCONFIG tc{};
		tc.cbSize = sizeof(tc);
		tc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION;
		tc.pszMainIcon = TD_WARNING_ICON;
		tc.pszWindowTitle = L"NekroWare";
		tc.pszMainInstruction = L"Fishstrap is required";
		tc.pszContent = L"NekroWare found a Roblox client that was not launched through Fishstrap.\r\n"
			L"Without Fishstrap, NekroWare cannot determine the Roblox version and load the correct offsets.\r\n\r\n"
			L"Install Fishstrap, launch Roblox with it, and run NekroWare again.";
		tc.pButtons = buttons;
		tc.cButtons = 2;
		tc.nDefaultButton = 1;

		int result = 0;
		HRESULT hr = TaskDialogIndirect(&tc, &result, nullptr, nullptr);
		if (FAILED(hr) || result == 0) // no comctl32 v6 - fall back to a plain MessageBox
		{
			int mb = MessageBoxW(NULL,
				L"NekroWare requires Roblox to be installed/launched through Fishstrap, otherwise it cannot auto-load offsets.\n\n"
				L"Install Fishstrap (fishstrap.app) and run the cheat again.",
				L"NekroWare - Fishstrap required",
				MB_ICONWARNING | MB_YESNO);
			return mb == IDYES;
		}
		return result == 2;
	}

	// Downloads the offsets header for a version from offsets.imtheo.lol.
	inline std::string FetchOffsetsHpp(const std::string& version)
	{
		HINTERNET hSession = WinHttpOpen(L"NekroWare", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
			WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
		if (!hSession)
			return "";
		WinHttpSetTimeouts(hSession, 10000, 10000, 15000, 15000);

		HINTERNET hConnect = WinHttpConnect(hSession, L"offsets.imtheo.lol", INTERNET_DEFAULT_HTTPS_PORT, 0);
		if (!hConnect)
		{
			WinHttpCloseHandle(hSession);
			return "";
		}

		std::wstring path = L"/" + std::wstring(version.begin(), version.end()) + L"/offsets.hpp";
		HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), nullptr,
			WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
		if (!hRequest)
		{
			WinHttpCloseHandle(hConnect);
			WinHttpCloseHandle(hSession);
			return "";
		}

		std::string body;
		if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
			WinHttpReceiveResponse(hRequest, nullptr))
		{
			char buffer[4096];
			DWORD bytesRead = 0;
			while (WinHttpReadData(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0)
			{
				body.append(buffer, bytesRead);
				bytesRead = 0;
			}
		}

		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return body;
	}

	// Parses the downloaded header (same format as the static dump:
	// "namespace X { inline constexpr uintptr_t Member = 0x..; }") and
	// overrides the matching runtime variables. Missing members keep the
	// built-in fallback values.
	inline int ParseAndApply(const std::string& text)
	{
		int applied = 0;
		std::string currentNs;

		size_t pos = 0;
		while (pos < text.size())
		{
			size_t lineEnd = text.find('\n', pos);
			if (lineEnd == std::string::npos)
				lineEnd = text.size();
			std::string line = text.substr(pos, lineEnd - pos);
			pos = lineEnd + 1;

			if (line.empty())
				continue;

			// namespace X {
			if (line.find("namespace") != std::string::npos)
			{
				size_t open = line.find('{');
				size_t nameStart = line.find_first_not_of(" \t", line.find("namespace") + 9);
				if (open != std::string::npos && nameStart != std::string::npos)
				{
					std::string name = line.substr(nameStart, line.find_first_of(" \t{", nameStart) - nameStart);
					if (name != "Offsets")
						currentNs = name;
					else
						currentNs.clear();
				}
				continue;
			}

			if (line.find_first_not_of(" \t}") == std::string::npos)
			{
				currentNs.clear();
				continue;
			}

			// inline std::string ClientVersion = "version-...";
			if (line.find("ClientVersion") != std::string::npos)
			{
				size_t q1 = line.find('"');
				size_t q2 = q1 == std::string::npos ? std::string::npos : line.find('"', q1 + 1);
				if (q1 != std::string::npos && q2 != std::string::npos)
					Offsets::ClientVersion = line.substr(q1 + 1, q2 - q1 - 1);
				continue;
			}

			// inline [constexpr] uintptr_t Member = 0x..;
			size_t eq = line.find('=');
			if (currentNs.empty() || eq == std::string::npos)
				continue;

			size_t nameEnd = line.find_last_not_of(" \t", eq - 1);
			size_t nameStart = line.find_last_of(" \t", nameEnd);
			if (nameEnd == std::string::npos)
				continue;
			std::string member = line.substr(nameStart + 1, nameEnd - nameStart);

			size_t valStart = line.find("0x", eq);
			if (valStart == std::string::npos)
				continue;
			uintptr_t value = 0;
			try { value = std::stoull(line.substr(valStart, line.find_first_not_of("0123456789abcdefABCDEF", valStart + 2) - valStart), nullptr, 16); }
			catch (...) { continue; }

			std::string key = currentNs + "::" + member;
			auto it = g_OffsetRegistry.find(key);
			if (it != g_OffsetRegistry.end())
			{
				*(it->second) = value;
				applied++;
			}
		}
		return applied;
	}

	// Full pipeline: detect the running client's version, download the
	// matching offsets header, apply it. Exits NekroWare with an explanatory
	// dialog if the running Roblox is not launched through Fishstrap. Falls
	// back to the built-in static offsets (from rbx/offsets.h) on download
	// failures.
	inline void LoadOffsets()
	{
		std::string version = GetRunningRobloxVersion();
		if (version.empty())
		{
			std::cout << "[-] Could not detect a Fishstrap Roblox version from the process path" << std::endl;
			bool openSite = ConfirmFishstrapRequired();
			if (openSite)
			{
				std::cout << "[*] Opening https://fishstrap.app/ ..." << std::endl;
				ShellExecuteW(NULL, L"open", L"https://fishstrap.app/", NULL, NULL, SW_SHOWNORMAL);
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
			std::cout << "[-] NekroWare requires Fishstrap, exiting..." << std::endl;
			std::exit(0);
		}

		std::cout << "[*] Detected Roblox version: " << version << std::endl;
		std::cout << "[*] Fetching offsets from offsets.imtheo.lol..." << std::endl;

		std::string text = FetchOffsetsHpp(version);
		if (text.empty())
		{
			std::cout << "[-] Offsets download failed, using built-in offsets (version " << Offsets::ClientVersion << ")" << std::endl;
			return;
		}

		int applied = ParseAndApply(text);
		if (applied == 0)
		{
			std::cout << "[-] Offsets header was empty/invalid, using built-in offsets (version " << Offsets::ClientVersion << ")" << std::endl;
			return;
		}

		std::cout << "[+] Applied " << applied << " dynamic offsets for " << Offsets::ClientVersion << std::endl;
	}
}
