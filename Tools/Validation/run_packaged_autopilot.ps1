param(
    [string]$Exe = "",          # path to packaged EmberboundIsekai.exe (auto-detect if empty)
    [int]$MaxSeconds = 180,
    [string]$Tag = "packaged"
)
# M12: drive the full-loop autopilot through the PACKAGED standalone build (not the editor)
# and capture a rendered frame from the cooked game. Confirms the shipped build runs the loop
# end-to-end outside the editor. Stops only the PID it launches.
$ErrorActionPreference = "Continue"
$root = "E:\mobilegamer\EmberboundIsekai"
$outDir = "$root\Docs\validation"
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

if ($Exe -eq "") {
    $cand = Get-ChildItem "$root\Packaged" -Recurse -Filter "EmberboundIsekai.exe" -ErrorAction SilentlyContinue |
            Where-Object { $_.FullName -notmatch '\\Binaries\\' -or $_.FullName -match 'Windows' } |
            Sort-Object Length -Descending | Select-Object -First 1
    if ($cand) { $Exe = $cand.FullName }
}
if ($Exe -eq "" -or -not (Test-Path $Exe)) { Write-Output "PACKAGED_EXE_NOT_FOUND (searched $root\Packaged)"; exit 1 }
Write-Output "EXE=$Exe"

# packaged builds write logs/screenshots under the staged project Saved dir or %LOCALAPPDATA%
$args = @("/Game/Emberbound/Maps/Camp/L_Camp?game=/Script/EmberboundIsekai.CampGameMode",
          "-RenderOffScreen","-nosplash","-stdout","-resx=1280","-resy=720",
          "-ExecCmds=`"Ember.Autopilot 1,Ember.CaptureAfter 30 showui`"")
$p = Start-Process -FilePath $Exe -ArgumentList $args -PassThru
Write-Output "LAUNCHED pid=$($p.Id) tag=$Tag"
$deadline = (Get-Date).AddSeconds($MaxSeconds)
while ((Get-Date) -lt $deadline) {
    Start-Sleep -Seconds 3
    if ($p.HasExited) { break }
}
if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force }
Write-Output "STOPPED tag=$Tag"

# find the packaged log + newest screenshot
$logCandidates = @(
    "$env:LOCALAPPDATA\EmberboundIsekai\Saved\Logs\EmberboundIsekai.log",
    (Split-Path $Exe -Parent) + "\..\..\..\EmberboundIsekai\Saved\Logs\EmberboundIsekai.log"
)
foreach ($lc in $logCandidates) {
    if (Test-Path $lc) { Copy-Item $lc "$outDir\packaged_autopilot.log" -Force; Write-Output "LOG=$lc"; break }
}
$shot = Get-ChildItem "$env:LOCALAPPDATA\EmberboundIsekai\Saved\Screenshots" -Recurse -Filter *.png -ErrorAction SilentlyContinue |
        Sort-Object LastWriteTime | Select-Object -Last 1
if ($shot) { Copy-Item $shot.FullName "$outDir\packaged_camp.png" -Force; Write-Output "SHOT=$($shot.FullName)" } else { Write-Output "NO_PACKAGED_SHOT" }
Write-Output "DONE tag=$Tag"
