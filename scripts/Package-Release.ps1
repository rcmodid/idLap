[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$BuildDirectory,

    [Parameter(Mandatory = $true)]
    [string]$BootApp0Path,

    [Parameter(Mandatory = $true)]
    [string]$OutputDirectory,

    [Parameter(Mandatory = $true)]
    [string]$Version,

    [ValidateSet("c3_transponder", "c3_decoder_minimal", "c3_decoder_w5500", "c3_decoder_lengkap")]
    [string[]]$FirmwareProfiles = @("c3_transponder", "c3_decoder_minimal", "c3_decoder_w5500", "c3_decoder_lengkap")
)

$ErrorActionPreference = "Stop"

$profiles = @(
    @{ id = "c3_transponder"; name = "IDLap Transponder"; type = "transponder" },
    @{ id = "c3_decoder_minimal"; name = "idLap Decoder USB"; type = "decoder" },
    @{ id = "c3_decoder_w5500"; name = "idLap Decoder W5500"; type = "decoder" },
    @{ id = "c3_decoder_lengkap"; name = "idLap Decoder W5500 + OLED"; type = "decoder" }
) | Where-Object { $_.id -in $FirmwareProfiles }

$partDefinitions = @(
    @{ source = "bootloader.bin"; suffix = "bootloader"; address = "0x0" },
    @{ source = "partitions.bin"; suffix = "partitions"; address = "0x8000" },
    @{ source = "boot_app0.bin"; suffix = "boot_app0"; address = "0xe000" },
    @{ source = "firmware.bin"; suffix = "firmware"; address = "0x10000" }
)

New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null
$firmware = @()

foreach ($profile in $profiles) {
    $parts = @()
    foreach ($part in $partDefinitions) {
        $source = if ($part.source -eq "boot_app0.bin") {
            $BootApp0Path
        } else {
            Join-Path (Join-Path $BuildDirectory $profile.id) $part.source
        }
        if (-not (Test-Path -LiteralPath $source -PathType Leaf)) {
            throw "Firmware asset missing: $source"
        }

        $assetName = "{0}-{1}.bin" -f $profile.id, $part.suffix
        Copy-Item -LiteralPath $source -Destination (Join-Path $OutputDirectory $assetName) -Force
        $parts += [ordered]@{
            address = $part.address
            asset = $assetName
        }
    }

    $firmware += [ordered]@{
        id = $profile.id
        name = $profile.name
        type = $profile.type
        chipFamily = "ESP32-C3"
        parts = $parts
    }
}

$manifest = [ordered]@{
    schemaVersion = 1
    version = $Version
    generatedAt = (Get-Date).ToUniversalTime().ToString("o")
    firmware = $firmware
}

$manifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $OutputDirectory "manifest.json") -Encoding utf8

$checksumLines = Get-ChildItem -LiteralPath $OutputDirectory -Filter "*.bin" |
    Sort-Object Name |
    ForEach-Object {
        $hash = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
        "{0}  {1}" -f $hash, $_.Name
    }
$checksumLines | Set-Content -LiteralPath (Join-Path $OutputDirectory "SHA256SUMS") -Encoding ascii
