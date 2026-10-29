cmake --install .\build\windows-release --config Release
ctest --preset windows-installed

Copy-Item `
    -LiteralPath 'C:\code\AFSIM\afsim_2.2505.0\build\grammar' `
    -Destination 'C:\code\AFSIM\AFSIM' `
    -Recurse -Force 

