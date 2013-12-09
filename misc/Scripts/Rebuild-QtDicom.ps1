# This scripts rebuilds all configurations of Qt Flux solution. It requires
# the following environmental variables to be set:
#
# - %QTDIRBASE%, pointing to the root directory of 32- and 64-bit installations
#   of the Qt library, like: E:\Qt\4.8.1\msvc
# - %SDKDIR%, holding path to the root of the Windows SDK 7.1, e.g.
#   C:\Program Files\Microsoft SDKs\Windows\v7.1
#
#
# Author: Paweł Żak <pawel.zak@fluxinc.ca>

$CurrentDir = Split-Path $MyInvocation.MyCommand.Path
$ErrorActionPreference = "Stop"
$OutFile = "Build-All.txt"


# Resolve path to the solution file
$Solution = ""
try {
    $Solution = Resolve-Path ( Join-Path $CurrentDir "..\..\Qt Dicom.sln" )
    Write-Host "Solution path    :", $Solution
}
catch {
    Write-Error "Failed to resolve path to the Qt Flux solution"
    exit 1
}

# And to the Build-One batch from current dirrectory
$RebuildOne = ""
try {
    $RebuildOne = Resolve-Path ( Join-Path $CurrentDir "Rebuild-One.bat" )
    Write-Host "Rebuild-One path :", $RebuildOne
}
catch {
    Write-Error "Missing Rebuild-One batch file"
}


$Configurations = [String[]] @( 'Release', 'Debug' );
$Architectures = [String[]]  @( 'x86', 'x64' );

Write-Host "Qt libraries     : $env:QTDIRBASE"

foreach ( $Configuration in $Configurations ) {
    foreach ( $Architecture in $Architectures ) {
        Write-Host -NoNewline "`t$Configuration/$Architecture... "

        . $RebuildOne "$Solution" "$Architecture" "$Configuration" | Out-File $OutFile
        $Code = $LASTEXITCODE
        if ( $Code -eq 0 ) {
            Write-Host "Done."
            Remove-Item -Force $OutFile
        }
        else {
            Write-Host "Failed. Exit code: $Code"
            "Exit code: $Code" | Out-File -Append $OutFile
            exit 1
        }
    }
}

exit 0