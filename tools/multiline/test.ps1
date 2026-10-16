param([Parameter(Mandatory=$true)][string]$BoostRoot,[string]$AtlRoot,[switch]$Navigation)
$ErrorActionPreference='Stop'
$root=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
if(-not (Get-Command cl.exe -ErrorAction SilentlyContinue)){throw 'Run in an x64 VS Developer PowerShell.'}
$build=Join-Path $root 'build\multiline-tests'
$null=New-Item -ItemType Directory -Path $build -Force
Push-Location $build
try {
  $common=@('/nologo','/EHsc','/MT','/utf-8','/DUNICODE','/D_UNICODE','/DNDEBUG','/std:c++17',('/I'+(Join-Path $root 'include')),('/I'+(Join-Path $root 'WeaselUI')),('/I'+$BoostRoot))
  $libs=@(('/LIBPATH:'+(Join-Path $BoostRoot 'stage\lib')),'user32.lib','advapi32.lib','ole32.lib','gdi32.lib')
  if($AtlRoot){$common+=('/I'+(Join-Path $AtlRoot 'include'));$libs+=('/LIBPATH:'+(Join-Path $AtlRoot 'lib\x64'))}
  foreach($kind in @('ipc','layout')){
    $library=if($kind -eq 'ipc'){'WeaselIPC.lib'}else{'WeaselUI.lib'}
    $compilerArgs=$common+@((Join-Path $root ('tests\multiline\test_'+$kind+'.cpp')),('/Fe:test_'+$kind+'.exe'),'/link',(Join-Path $root ('msbuild\Release\x64\'+$library)))+$libs
    & cl.exe @compilerArgs
    if($LASTEXITCODE){throw "Compilation failed: $kind"}
    & (Join-Path $build ('test_'+$kind+'.exe'))
    if($LASTEXITCODE){throw "Test failed: $kind"}
  }
  if($Navigation){& (Join-Path $build 'test_ipc.exe') --navigation;if($LASTEXITCODE){throw 'Navigation test failed'}}
} finally {Pop-Location}
