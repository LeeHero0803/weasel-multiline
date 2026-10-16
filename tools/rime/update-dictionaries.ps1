param(
  [string]$UserDir=(Join-Path $env:APPDATA 'Rime'),
  [string]$CacheDir=(Join-Path $PSScriptRoot '.cache\rime-ice'),
  [string]$SourceRepository='https://github.com/iDvel/rime-ice.git',
  [string]$BaselinePath=(Join-Path $PSScriptRoot '..\..\rime-integration\dictionary-baseline.json'),
  [switch]$Apply
)
$ErrorActionPreference='Stop'
$userRoot=(Resolve-Path -LiteralPath $UserDir).Path.TrimEnd('\')
$cache=[IO.Path]::GetFullPath($CacheDir)
function Git([string[]]$Arguments){
  $result=& git.exe @Arguments
  if($LASTEXITCODE){throw ('git failed: '+($Arguments -join ' '))}
  return $result
}
function Hash([string]$Path){
  $stream=[IO.File]::OpenRead($Path)
  $sha=[Security.Cryptography.SHA256]::Create()
  try{return [BitConverter]::ToString($sha.ComputeHash($stream)).Replace('-','').ToLowerInvariant()}
  finally{$stream.Dispose();$sha.Dispose()}
}
function Target([string]$Relative){
  $path=[IO.Path]::GetFullPath((Join-Path $userRoot $Relative))
  if(-not $path.StartsWith($userRoot+'\',[StringComparison]::OrdinalIgnoreCase)){throw 'Unsafe dictionary path'}
  return $path
}
if(Test-Path -LiteralPath (Join-Path $cache '.git')){
  if(Git -Arguments @('-C',$cache,'status','--porcelain')){throw 'Upstream cache has local changes; resolve them first.'}
  $null=Git -Arguments @('-C',$cache,'fetch','--prune','origin')
  $null=Git -Arguments @('-C',$cache,'pull','--ff-only')
}else{
  $null=New-Item -ItemType Directory -Path ([IO.Path]::GetDirectoryName($cache)) -Force
  $null=Git -Arguments @('clone','--single-branch',$SourceRepository,$cache)
}
$upstream=(Git -Arguments @('-C',$cache,'rev-parse','HEAD')).Trim()
$statePath=Join-Path $userRoot '.dictionary-upstream-state.json'
$baselineFile=if(Test-Path -LiteralPath $statePath){$statePath}else{$BaselinePath}
$baseline=(Get-Content -LiteralPath $baselineFile -Raw | ConvertFrom-Json).files
$sourceFiles=@{}
foreach($directory in @('cn_dicts','en_dicts')){
  $folder=Join-Path $cache $directory
  if(Test-Path -LiteralPath $folder){
    foreach($file in (Get-ChildItem -LiteralPath $folder -File)){
      $sourceFiles[$directory+'/'+$file.Name]=$file.FullName
    }
  }
}
foreach($name in @('rime_ice.dict.yaml','melt_eng.dict.yaml')){
  if(Test-Path -LiteralPath (Join-Path $cache $name)){$sourceFiles[$name]=Join-Path $cache $name}
}
if(-not $sourceFiles.Count){throw 'No allowed upstream dictionary files found.'}
$plan=@();$conflicts=@();$hashes=@{}
foreach($relative in ($sourceFiles.Keys | Sort-Object)){
  $target=Target $relative
  $hashes[$relative]=Hash $sourceFiles[$relative]
  $exists=Test-Path -LiteralPath $target
  $current=if($exists){Hash $target}else{$null}
  if($current -eq $hashes[$relative]){continue}
  $known=$baseline.PSObject.Properties[$relative]
  if($exists -and (-not $known -or $current -ne $known.Value)){$conflicts+=$relative}
  $plan+=@{relative=$relative;source=$sourceFiles[$relative];target=$target;existed=$exists}
}
Write-Output ('Upstream commit: '+$upstream)
Write-Output ('Changed/new files: '+$plan.Count)
$plan | ForEach-Object {Write-Output $_.relative}
if($conflicts.Count){throw ('Locally modified dictionaries need manual merge: '+($conflicts -join ', '))}
if(-not $Apply){Write-Output 'Preview only. Re-run with -Apply to back up and copy these files.';return}
$backup=Join-Path $userRoot ('.dictionary-backups\'+(Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
foreach($item in $plan){
  if($item.existed){
    $copy=Join-Path $backup $item.relative
    $null=New-Item -ItemType Directory -Path ([IO.Path]::GetDirectoryName($copy)) -Force
    Copy-Item -LiteralPath $item.target -Destination $copy
  }
}
foreach($item in $plan){
  $null=New-Item -ItemType Directory -Path ([IO.Path]::GetDirectoryName($item.target)) -Force
  Copy-Item -LiteralPath $item.source -Destination $item.target -Force
}
$state=@{repository=$SourceRepository;commit=$upstream;files=$hashes}
[IO.File]::WriteAllText($statePath,($state | ConvertTo-Json -Depth 5),(New-Object Text.UTF8Encoding($false)))
Write-Output ('Copied dictionaries. Backup: '+$backup)
Write-Output 'Run Weasel Deploy to compile the updated dictionaries.'
