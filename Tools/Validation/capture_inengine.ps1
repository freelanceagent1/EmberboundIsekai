param(
    [string]$Map = "/Game/Emberbound/Maps/Camp/L_Camp",
    [string]$Setup = "",       # extra ExecCmds run before the shot
    [int]$ShotDelayFrames = 1, # HighResShot fires at BeginPlay; scene actors already spawned
    [int]$RunSeconds = 18,
    [string]$Tag = "run"
)
# Renders the real game viewport off-screen (no desktop needed) and writes a PNG
# via UE HighResShot. Works in headless/RDP sessions. Stops only the process it starts.
$ErrorActionPreference = "Continue"
$proj = "E:\mobilegamer\EmberboundIsekai\EmberboundIsekai.uproject"
$editor = "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
$shotDir = "E:\mobilegamer\EmberboundIsekai\Saved\Screenshots\WindowsEditor"
$outDir = "E:\mobilegamer\EmberboundIsekai\Docs\validation"
New-Item -ItemType Directory -Force -Path $outDir | Out-Null
# clear old shots so we can identify this run's output
if (Test-Path $shotDir) { Get-ChildItem $shotDir -Filter *.png | Remove-Item -Force -ErrorAction SilentlyContinue }

$exec = @()
if ($Setup -ne "") { $exec += $Setup }
$exec += "HighResShot 1280x720"
$execStr = ($exec -join ",")

$args = @("`"$proj`"", $Map, "-game", "-RenderOffScreen", "-unattended", "-nopause",
          "-nosplash", "-stdout", "-windowed", "-resx=1280", "-resy=720",
          "-ExecCmds=`"$execStr`"")
$p = Start-Process -FilePath $editor -ArgumentList $args -PassThru
Write-Output "LAUNCHED pid=$($p.Id) tag=$Tag exec=[$execStr]"
Start-Sleep -Seconds $RunSeconds
if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force }
Start-Sleep -Seconds 1

# copy whatever shot(s) were produced to the validation dir with the tag
$shots = Get-ChildItem $shotDir -Filter *.png -ErrorAction SilentlyContinue | Sort-Object LastWriteTime
$i = 0
foreach ($s in $shots) {
    $dest = Join-Path $outDir ("{0}_{1}.png" -f $Tag, $i)
    Copy-Item $s.FullName $dest -Force
    Write-Output "SHOT $dest ($([math]::Round($s.Length/1KB)) KB)"
    $i++
}
if ($i -eq 0) { Write-Output "NO_SHOT tag=$Tag" }
Write-Output "DONE tag=$Tag"
