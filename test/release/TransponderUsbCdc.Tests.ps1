$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$platformIoConfig = Join-Path $repoRoot "platformio.ini"

Describe "Transponder USB CDC build configuration" {
    It "routes Serial through ESP32-C3 native USB CDC at boot" {
        $config = Get-Content -LiteralPath $platformIoConfig -Raw
        $transponderSection = [regex]::Match(
            $config,
            '(?ms)^\[env:c3_transponder\](.*?)(?=^\[|\z)'
        ).Groups[1].Value

        $transponderSection | Should Match '(?m)^\s*-DARDUINO_USB_MODE=1\s*$'
        $transponderSection | Should Match '(?m)^\s*-DARDUINO_USB_CDC_ON_BOOT=1\s*$'
    }
}
