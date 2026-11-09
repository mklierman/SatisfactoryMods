param (
    [string]$SingleFolder = ""
)

$SMLModsPath = "C:\Modding\SatisfactoryModLoader\Mods"
$SMLPath = $SMLModsPath + "\GameFeatures"
$ModDir = "C:\Modding\SatisfactoryMods"

function New-Sym-Link ($Link) {
    $LinkRoot = $SMLPath
    if ($Link -eq "AccessTransformerEditor") {
        $LinkRoot = $SMLModsPath
        Remove-Item ($SMLPath + "\" + $Link) -ErrorAction SilentlyContinue -Recurse
    }

    Remove-Item ($LinkRoot + "\" + $Link) -ErrorAction SilentlyContinue -Recurse
    New-Item -Path ($LinkRoot + "\" + $Link) -ItemType Junction -Value ($ModDir + "\" + $Link) -Force
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
