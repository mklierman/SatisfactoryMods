param (
    [string]$SingleFolder = ""
)

$SMLPath = "C:\Code\SatisfactoryModLoader-1.0\Mods"
$ModDir = "C:\Code\SF\SatisfactoryMods"

function New-Sym-Link ($Link) {
    Remove-Item ($SMLPath + "\" + $Link) -ErrorAction SilentlyContinue -Recurse
    New-Item -Path ($SMLPath + "\" + $Link) -ItemType SymbolicLink -Value ($ModDir + "\" + $Link) -Force
}

if ($SingleFolder -ne "") {
    $dir = Get-ChildItem $ModDir | Where-Object {$_.PSISContainer -and $_.Name -eq $SingleFolder}
    if ($dir) {
        Write-Output "Processing folder: $($dir.Name)"
        New-Sym-Link $dir.Name
    }
    else {
        Write-Output "Folder '$SingleFolder' not found."
    }
}
else {
    $dir = Get-ChildItem $ModDir | Where-Object {$_.PSISContainer}
    foreach ($d in $dir) {
        Write-Output "Processing folder: $($d.Name)"
        New-Sym-Link $d.Name
    }
}