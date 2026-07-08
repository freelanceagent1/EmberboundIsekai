$ErrorActionPreference = "Continue"
$exe = "E:\mobilegamer\EmberboundIsekai\Packaged\Windows\EmberboundIsekai.exe"
$log = "E:\mobilegamer\EmberboundIsekai\Docs\validation\M12_packaged_autopilot.log"
$out = "E:\mobilegamer\EmberboundIsekai\Docs\validation"
if (Test-Path $log) { Remove-Item $log -Force }
if (-not (Test-Path $exe)) { Write-Output "NO_EXE"; exit 1 }
$args = @("/Game/Emberbound/Maps/Camp/L_Camp?game=/Script/EmberboundIsekai.CampGameMode",
          "-RenderOffScreen","-nosplash","-stdout","-resx=1280","-resy=720",
          "-ExecCmds=`"Ember.Autopilot 1,Ember.CaptureAfter 30 showui`"",
          "-AbsLog=`"$log`"")
$p = Start-Process -FilePath $exe -ArgumentList $args -PassThru
Write-Output "LAUNCHED pid=$($p.Id)"
$deadline = (Get-Date).AddSeconds(220)
$complete = $false
while ((Get-Date) -lt $deadline) {
    Start-Sleep -Seconds 5
    if (Test-Path $log) {
        if (Select-String -Path $log -Pattern "AUTOPILOT COMPLETE" -SimpleMatch -Quiet) { $complete = $true; Start-Sleep -Seconds 3; break }
    }
    if ($p.HasExited) { break }
}
# stop only the packaged processes we launched (bootstrap + child)
Get-Process EmberboundIsekai -ErrorAction SilentlyContinue | Where-Object { $_.StartTime -ge $p.StartTime.AddSeconds(-2) } | Stop-Process -Force -ErrorAction SilentlyContinue
Start-Sleep -Seconds 1
Write-Output "COMPLETE_MARKER=$complete"
# copy newest packaged screenshot
$shotDirs = @("E:\mobilegamer\EmberboundIsekai\Packaged\Windows\EmberboundIsekai\Saved\Screenshots",
              "$env:LOCALAPPDATA\EmberboundIsekai\Saved\Screenshots")
foreach ($sd in $shotDirs) {
    $s = Get-ChildItem $sd -Recurse -Filter *.png -ErrorAction SilentlyContinue | Sort-Object LastWriteTime | Select-Object -Last 1
    if ($s) { Copy-Item $s.FullName "$out\packaged_autopilot.png" -Force; Write-Output "SHOT=$($s.FullName)"; break }
}
Write-Output "DONE"
