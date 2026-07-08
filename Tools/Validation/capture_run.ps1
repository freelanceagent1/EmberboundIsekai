param(
    [string]$Map = "/Game/Emberbound/Maps/Camp/L_Camp",
    [string]$ExecCmds = "",
    [int[]]$ShotAtSeconds = @(12, 25, 40),
    [int]$KillAfterSeconds = 50,
    [string]$Tag = "run"
)
# Launches EmberboundIsekai windowed (-game, real RHI), captures timed full-screen
# screenshots to Docs/validation/, then stops ONLY the process it launched.
$ErrorActionPreference = "Continue"
$proj = "E:\mobilegamer\EmberboundIsekai\EmberboundIsekai.uproject"
$editor = "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
$outDir = "E:\mobilegamer\EmberboundIsekai\Docs\validation"
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing
function Save-Screen([string]$path) {
    $b = [System.Windows.Forms.SystemInformation]::VirtualScreen
    $bmp = New-Object System.Drawing.Bitmap $b.Width, $b.Height
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.CopyFromScreen($b.Left, $b.Top, 0, 0, $bmp.Size)
    $bmp.Save($path, [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $bmp.Dispose()
}

$args = @("`"$proj`"", $Map, "-game", "-windowed", "-resx=1280", "-resy=720", "-log")
if ($ExecCmds -ne "") { $args += "-ExecCmds=`"$ExecCmds`"" }
$p = Start-Process -FilePath $editor -ArgumentList $args -PassThru
Write-Output "LAUNCHED pid=$($p.Id) tag=$Tag"

$elapsed = 0
$shots = $ShotAtSeconds | Sort-Object
foreach ($t in $shots) {
    Start-Sleep -Seconds ($t - $elapsed)
    $elapsed = $t
    $file = Join-Path $outDir ("{0}_t{1}s.png" -f $Tag, $t)
    Save-Screen $file
    Write-Output "SHOT $file"
}
if ($KillAfterSeconds -gt $elapsed) { Start-Sleep -Seconds ($KillAfterSeconds - $elapsed) }
if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force }
Write-Output "DONE tag=$Tag exited=$($p.HasExited)"
