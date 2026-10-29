Get-Content -LiteralPath "$PSScriptRoot\build\windows-release\install_manifest.txt" -ErrorAction Stop |
    ForEach-Object {
        if (Test-Path -LiteralPath $_) {
            Remove-Item -LiteralPath $_ #-WhatIf
        }
    }
