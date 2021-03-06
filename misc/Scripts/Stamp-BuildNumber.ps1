Param (
  [Parameter(Mandatory=$True)][String] $Path,
  [String] $Source = "FILE:D:\FPD\src\fpduip\Resources\Build Number.txt",
  [RegEx] $Pattern = "\d+[,.]\d+[,.]\d+[,.](\d+)"
)


function parseSource() {
    $result = ""
    if ( $Source.StartsWith( "DIRECT:" ) ) {
        $result = $Source.Substring( 7 )
    }
    elseif ( $Source.StartsWith( "FILE:" ) ) {
        Get-Content $Source.Substring( 5 ) | % { $result += $_ -replace "\D","" }
    }
    
    $result
}


function replaceBuildNumber( [String] $BuildNumber, [Ref] $changed ) {
    $changed.Value = $False
    
    $contents = ""
    Get-Content -Encoding "UTF8" $Path | % {
        $Line = $_
        if ( -not $Pattern.IsMatch( $Line ) ) {
            $contents += $Line
        }
        else {
            $Group = $Pattern.Match( $Line ).Groups[ 1 ]
            
            $NewLine = $Line.Substring( 0, $Group.Index )
            $NewLine += $BuildNumber
            $NewLine += $Line.Substring( $Group.Index + $Group.Length )
            
            if ( $NewLine -ne $Line ) {
                $changed.Value = $True
                $contents += $NewLine
            }
            else {
                $contents += $Line
            }
        }
        $contents += "`r`n"
    }
    
    $contents.Substring( 0, $contents.Length - 2 )
}

try {
    $BuildNumber = parseSource
    
    $changed = $False
    $Contents = replaceBuildNumber $BuildNumber ([Ref] $changed)
    
    if ( $Changed ) {
        Write-Host "Stamping with build number:", $BuildNumber
        
        [System.IO.File]::WriteAllLines( $Path, $Contents )    
    }
    else {
        Write-Host "File is already at version:", $BuildNumber
    }
}
catch {
    Write-Host "Failed to stamp build number"

    exit 1
}
