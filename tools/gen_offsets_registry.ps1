# Regenerates the runtime offset registry after replacing rbx/offsets.h with a
# fresh dump. Also converts the dump's `inline constexpr uintptr_t` members to
# `inline uintptr_t` so the values can be overridden at startup from the
# dynamically downloaded header for the running client version.
#
# Usage:  powershell -ExecutionPolicy Bypass -File tools\gen_offsets_registry.ps1
# Run from the repo root.

$ErrorActionPreference = 'Stop'

$header = Join-Path $PSScriptRoot '..\NekroWare\rbx\offsets.h'
$reg    = Join-Path $PSScriptRoot '..\NekroWare\rbx\offsets_registry.h'

$content = Get-Content -LiteralPath $header -Raw

# 1) Convert constexpr members to runtime variables (values kept as fallback).
$converted = $content -replace 'inline constexpr uintptr_t', 'inline uintptr_t'
if ($converted -ne $content)
{
    Set-Content -LiteralPath $header -Value $converted -NoNewline -Encoding ascii
    Write-Host "[+] Converted offsets.h members to runtime variables"
}
else
{
    Write-Host "[=] offsets.h members already runtime variables"
}

# 2) Generate the registry: one OFFSET_REG(namespace, member) line per member.
$entries = [System.Collections.Generic.List[string]]::new()
[regex]::Matches($converted, 'namespace\s+(\w+)\s*\{([^}]*)\}') | ForEach-Object {
    $ns = $_.Groups[1].Value
    if ($ns -eq 'Offsets') { return } # outer wrapper namespace, skip
    [regex]::Matches($_.Groups[2].Value, 'uintptr_t\s+(\w+)\s*=') | ForEach-Object {
        $entries.Add("OFFSET_REG($ns, $($_.Groups[1].Value))")
    }
}

if ($entries.Count -eq 0)
{
    Write-Host "[-] No entries generated - is the dump format still valid?" -ForegroundColor Red
    exit 1
}

$body = "// AUTO-GENERATED - run tools\gen_offsets_registry.ps1 after replacing rbx\offsets.h with a fresh dump.`r`n"
$body += $entries -join "`r`n"
$body += "`r`n"
Set-Content -LiteralPath $reg -Value $body -Encoding ascii

Write-Host "[+] Wrote $($entries.Count) registry entries to NekroWare\rbx\offsets_registry.h"
