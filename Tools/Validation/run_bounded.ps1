param(
    [string]$Map,
    [string]$ExecCmds,
    [string]$Tag,
    [string]$UntilPattern = "",   # kill early once this regex appears in the log
    [int]$MaxSeconds = 45
)
# Headless -game -nullrhi bounded run. Kills when UntilPattern seen or MaxSeconds.
$ErrorActionPreference = "Continue"
$proj = "E:\mobilegamer\EmberboundIsekai\EmberboundIsekai.uproject"
# Use full editor in -game mode (not -Cmd): the commandlet host does not run the
# gameplay enemy spawner correctly headlessly, whereas UnrealEditor.exe -game does.
$editor = "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
$log = "E:\mobilegamer\EmberboundIsekai\Docs\validation\$Tag.log"
if (Test-Path $log) { Remove-Item $log -Force }
$args = @("`"$proj`"", $Map, "-game", "-nullrhi", "-unattended", "-nopause", "-nosplash",
          "-stdout", "-ExecCmds=`"$ExecCmds`"", "-AbsLog=`"$log`"")
$p = Start-Process -FilePath $editor -ArgumentList $args -PassThru
Write-Output "LAUNCHED pid=$($p.Id) tag=$Tag"
$deadline = (Get-Date).AddSeconds($MaxSeconds)
$hit = $false
while ((Get-Date) -lt $deadline) {
    Start-Sleep -Seconds 2
    if ($p.HasExited) { break }
    if ($UntilPattern -ne "" -and (Test-Path $log)) {
        if (Select-String -Path $log -Pattern $UntilPattern -Quiet) { $hit = $true; Start-Sleep -Seconds 2; break }
    }
}
if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force }
Write-Output "STOPPED tag=$Tag until_hit=$hit"
