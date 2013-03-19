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
    $Root = Resolve-Path "..\.."
}
catch {
    Write-Error "Failed to resolve project root path"
    exit 1
}

( "amd64", "x86" ) | % {
    [String] $Arch = $_
    [String] $Dst = "$DstBase\$Arch"
    
    New-Item -type Directory -force $Dst | Out-Null
    
    New-Item -type Directory -force "$Dst\lib" | Out-Null
    ( "QtDicom4", "QtDicomd4" ) | % {
        $BaseName = $_
        ( "lib", "dll", "pdb" ) | % {
            Copy-Item -force "$Root\lib\$Arch\$BaseName.$_" "$Dst\lib" | Out-Null
        }
    }
    
    New-Item -type Directory -force "$Dst\bin" | Out-Null
    ( "QtDicom4", "QtDicomd4" ) | % {
        $BaseName = $_
        ( "lib", "dll", "pdb" ) | % {
            Copy-Item -force "$Root\lib\$Arch\$BaseName.$_" "$Dst\bin" | Out-Null
        }
    }
    
    New-Item -type Directory -force "$Dst\include" | Out-Null
    New-Item -type Directory -force "$Dst\include\QtDicom" | Out-Null
    
    Copy-Item -force "$Root\src\QtDicom\*" "$Dst\include\QtDicom" `
        -exclude ( "*.cpp", "*.moc.inl", "*.vcxproj*", "Resources" ) | Out-Null
}

Write-Host "Done"
