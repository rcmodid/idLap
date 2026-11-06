$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$packageScript = Join-Path $repoRoot "scripts\Package-Release.ps1"

Describe "Package-Release" {
    It "packages all three decoder variants without transponder assets" {
        $buildRoot = Join-Path $TestDrive "decoder-build"
        $outputRoot = Join-Path $TestDrive "decoder-release"
        $bootApp0Path = Join-Path $TestDrive "decoder-boot_app0.bin"
        $decoderProfiles = @("c3_decoder_minimal", "c3_decoder_w5500", "c3_decoder_lengkap")
        foreach ($profile in $decoderProfiles) {
            $directory = Join-Path $buildRoot $profile
            New-Item -ItemType Directory -Path $directory -Force | Out-Null
            foreach ($name in @("bootloader.bin", "partitions.bin", "firmware.bin")) {
                [System.IO.File]::WriteAllBytes((Join-Path $directory $name), [byte[]](1, 2, 3))
            }
        }
        [System.IO.File]::WriteAllBytes($bootApp0Path, [byte[]](4, 5, 6))
        & $packageScript -BuildDirectory $buildRoot -BootApp0Path $bootApp0Path -OutputDirectory $outputRoot -Version "v1.1.0" -FirmwareProfiles $decoderProfiles
        $manifest = Get-Content (Join-Path $outputRoot "manifest.json") -Raw | ConvertFrom-Json
        @($manifest.firmware).Count | Should Be 3
        @($manifest.firmware | Where-Object { $_.type -ne "decoder" }).Count | Should Be 0
        ($manifest.firmware.id -join ",") | Should Be "c3_decoder_minimal,c3_decoder_w5500,c3_decoder_lengkap"
        @(Get-ChildItem -LiteralPath $outputRoot -Filter "*.bin").Count | Should Be 12
        @(Get-ChildItem -LiteralPath $outputRoot -Filter "*transponder*").Count | Should Be 0
        @(Get-Content (Join-Path $outputRoot "SHA256SUMS")).Count | Should Be 12
    }

    It "packages only the selected transponder profile without requiring decoder builds" {
        $buildRoot = Join-Path $TestDrive "transponder-build"
        $profileDir = Join-Path $buildRoot "c3_transponder"
        $outputRoot = Join-Path $TestDrive "transponder-release"
        $bootApp0Path = Join-Path $TestDrive "transponder-boot_app0.bin"
        New-Item -ItemType Directory -Path $profileDir -Force | Out-Null
        foreach ($name in @("bootloader.bin", "partitions.bin", "firmware.bin")) {
            [System.IO.File]::WriteAllBytes((Join-Path $profileDir $name), [byte[]](1, 2, 3))
        }
        [System.IO.File]::WriteAllBytes($bootApp0Path, [byte[]](4, 5, 6))

        & $packageScript -BuildDirectory $buildRoot -BootApp0Path $bootApp0Path -OutputDirectory $outputRoot -Version "v1.0.3" -FirmwareProfiles c3_transponder

        $manifest = Get-Content (Join-Path $outputRoot "manifest.json") -Raw | ConvertFrom-Json
        @($manifest.firmware).Count | Should Be 1
        $manifest.firmware[0].id | Should Be "c3_transponder"
        @(Get-ChildItem -LiteralPath $outputRoot -Filter "*.bin").Count | Should Be 4
        @(Get-Content (Join-Path $outputRoot "SHA256SUMS")).Count | Should Be 4
    }

    It "creates an ESP Web Tools manifest and SHA-256 checksums for every firmware profile" {
        $buildRoot = Join-Path $TestDrive "build"
        $outputRoot = Join-Path $TestDrive "release"
        $bootApp0Path = Join-Path $TestDrive "boot_app0.bin"
        $profiles = @(
            "c3_transponder",
            "c3_decoder_minimal",
            "c3_decoder_w5500",
            "c3_decoder_lengkap"
        )

        foreach ($profile in $profiles) {
            $profileDir = Join-Path $buildRoot $profile
            New-Item -ItemType Directory -Path $profileDir -Force | Out-Null
            [System.IO.File]::WriteAllBytes((Join-Path $profileDir "bootloader.bin"), [byte[]](1, 2))
            [System.IO.File]::WriteAllBytes((Join-Path $profileDir "partitions.bin"), [byte[]](3, 4))
            [System.IO.File]::WriteAllBytes((Join-Path $profileDir "firmware.bin"), [byte[]](7, 8))
        }
        [System.IO.File]::WriteAllBytes($bootApp0Path, [byte[]](5, 6))

        & $packageScript -BuildDirectory $buildRoot -BootApp0Path $bootApp0Path -OutputDirectory $outputRoot -Version "v1.2.3"
        $? | Should Be $true

        $manifest = Get-Content (Join-Path $outputRoot "manifest.json") -Raw | ConvertFrom-Json
        $manifest.version | Should Be "v1.2.3"
        $manifest.firmware.Count | Should Be 4
        $manifest.firmware[0].parts[0].address | Should Be "0x0"
        $manifest.firmware[0].parts[1].address | Should Be "0x8000"
        $manifest.firmware[0].parts[2].address | Should Be "0xe000"
        $manifest.firmware[0].parts[3].address | Should Be "0x10000"

        $checksums = Get-Content (Join-Path $outputRoot "SHA256SUMS")
        $checksums.Count | Should Be 16
        ($checksums -join "`n") | Should Match "c3_transponder-bootloader.bin"
        ($checksums -join "`n") | Should Match "c3_decoder_lengkap-firmware.bin"
    }
}
