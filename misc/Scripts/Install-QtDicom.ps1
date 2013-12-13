# This scripts installs source and binary files belonging to the Qt DICOM project
# into the directory pointed out by the first parameter.
#
# Note, that you should successfully build the Qt DICOM project first in order to 
# use this script.
#
#
# Author: Paweł Żak <pawel.zak@fluxinc.ca>

Param(
    [Parameter(Mandatory=$True)][String] $path
)

[String] $DstBase = ""


$ErrorActionPreference = "Stop"
try {
    $DstBase = Resolve-Path $path
    Write-Host "Installing to", $DstBase
}
catch {
    Write-Error "Failed to resolve path: $Path"
    exit 1
}

try {
    $CurrentDir = Split-Path $MyInvocation.MyCommand.Path
    $Root = Resolve-Path ( Join-Path $CurrentDir "..\.." )
}
catch {
    Write-Error "Failed to resolve project root path"
    exit 1
}

( "amd64", "x86" ) | % {
    [String] $Arch = $_
    [String] $Dst = "$DstBase\$Arch"
    
    New-Item -ItemType Directory -Force -Path $Dst | Out-Null
    
    New-Item -ItemType Directory -Force "$Dst\lib" | Out-Null
    ( "QtDicom4", "QtDicomd4" ) | % {
        $BaseName = $_
        ( "lib", "dll", "pdb", "exp" ) | % {
            Copy-Item -force "$Root\lib\$Arch\$BaseName.$_" "$Dst\lib" | Out-Null
        }
    }
    
    New-Item -ItemType Directory -Force "$Dst\bin" | Out-Null
    ( "QtDicom4", "QtDicomd4" ) | % {
        $BaseName = $_
        ( "dll", "pdb" ) | % {
            Copy-Item -Force "$Root\lib\$Arch\$BaseName.$_" "$Dst\bin" | Out-Null
        }
    }
    
    New-Item -ItemType Directory -Force "$Dst\include" | Out-Null
    New-Item -ItemType Directory -Force "$Dst\include\QtDicom" | Out-Null
    
    Copy-Item -Force "$Root\src\QtDicom\*" "$Dst\include\QtDicom" `
        -Exclude ( "*.cpp", "*.moc.inl", "*.vcxproj*", "Resources" ) | Out-Null
}

Write-Host "Done"
exit 0