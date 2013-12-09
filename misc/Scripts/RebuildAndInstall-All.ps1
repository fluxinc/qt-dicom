# This script rebuilds and installs Qt Dicom project in all available Qt 
# releases.
#
# This script assumes that Qt libraries are built under the same root directory,
# e.g. C:\Libs\Qt and they all follow the following naming pattern:
#
#     <prefix>\<version>\msvc10\<architeture>
#
# E.g. C:\Libs\Qt\4.8.5\msvc10\amd64
#
#
# Author: Paweł Żak <pawel.zak@fluxinc.ca>

Param(
    [Parameter(Mandatory=$True)][String] $QtDirPrefix,
    [Parameter(Mandatory=$True)][String] $OutputDirPrefix,
    [Parameter(Mandatory=$True)][String[]] $Versions = @( "4.8.5", "4.8.1" )
)
$ErrorActionPreference = "Stop"

$CurrentDir = Split-Path $MyInvocation.MyCommand.Path
$VersionFilePath = Join-Path $CurrentDir "..\..\src\QtDicom\Version.hpp"
try {
    $VersionFilePath = Resolve-Path $VersionFilePath
}
catch {
    Write-Error "Failed to resolve version file path"
    exit 1;
}



function stampQtVersion(
    [String] $Version,
    [String] $VersionFilePath
) {
    $Digits = $Version -split '[.]'
    if ( $Digits.Count -ne 3 ) {
        Write-Host $Digits[ 3 ]
        Write-Error "Invalid version: $Version (digit count = $( $Digits.Count ))"
        
        exit 1;
    }

    $Patterns = @{
        "(\d+)[,.]\d+[,.]\d+[,.]\d+" = $Digits[ 0 ];
        "\d+[,.](\d+)[,.]\d+[,.]\d+" = $Digits[ 1 ];
        "\d+[,.]\d+[,.](\d+)[,.]\d+" = $Digits[ 2 ]
    }
        
    $StampNumber = Join-Path $CurrentDir "Stamp-BuildNumber.ps1"
    $Patterns.GetEnumerator() | % {
        $Pattern = $_.Key
        $Number = $_.Value

        .  $StampNumber -Path $VersionFilePath -Source "DIRECT:$Number" -Pattern $Pattern

        if ( $? ) {    
        }
        else {
            Write-Error "Failed to stamp version file $VersionFilePath with $Version"
            exit 1
        }
    }
}


foreach ( $Version in $Versions ) {
    $QtDirBase        = Join-Path ( Join-Path $QtDirPrefix        $Version ) "msvc10"
    $OutputDir        = Join-Path ( Join-Path $OutputDirPrefix    $Version ) "msvc10"

    if ( -not ( Test-Path $OutputDir ) ) {
        New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
    }

    try {
        $OutputDir            = Resolve-Path $OutputDir
        $env:QTDIRBASE        = Resolve-Path $QtDirBase
    }
    catch {
        Write-Error "Failed to resolve Qt path"
        exit 1
    }

    # Stamp the Qt version first...
    stampQtVersion $Version $VersionFilePath


    # ...then rebuild...    
    . ( Join-Path $CurrentDir "Rebuild-QtDicom.ps1" )
    if ( $? ) {

        # ..and install
        . ( Join-Path $CurrentDir "Install-QtDicom.ps1" ) $OutputDir
        if ( -not $? ) {
            Write-Error "Failed to install Qt DICOM solution"
            exit 1
        }
    }
    else {
        Write-Error "Failed to build Qt DICOM solution"
        exit 1
    }
}

exit 0
