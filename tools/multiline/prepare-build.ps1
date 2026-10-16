param(
  [Parameter(Mandatory=$true)][string]$BoostRoot,
  [Parameter(Mandatory=$true)][string]$LibrimeX86,
  [Parameter(Mandatory=$true)][string]$LibrimeX64
)
$ErrorActionPreference='Stop'
$root=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$boost=(Resolve-Path -LiteralPath $BoostRoot).Path
$x86=(Resolve-Path -LiteralPath $LibrimeX86).Path
$x64=(Resolve-Path -LiteralPath $LibrimeX64).Path
foreach($path in @((Join-Path $x86 'include\rime_api.h'),(Join-Path $x86 'lib\rime.lib'),(Join-Path $x64 'lib\rime.lib'))){
  if(-not (Test-Path -LiteralPath $path)){throw "Missing dependency: $path"}
}
$props=Get-Content -LiteralPath (Join-Path $root 'weasel.props.template') -Raw
$values=@{BOOST_ROOT=[Security.SecurityElement]::Escape($boost);PLATFORM_TOOLSET='v143';VERSION_MAJOR='0';VERSION_MINOR='17';VERSION_PATCH='4';PRODUCT_VERSION='0.17.4.3';FILE_VERSION='&quot;0.17.4.3&quot;'}
foreach($key in $values.Keys){$props=$props.Replace('$'+$key,$values[$key])}
[IO.File]::WriteAllText((Join-Path $root 'weasel.props'),$props,(New-Object Text.UTF8Encoding($false)))
Get-ChildItem -LiteralPath (Join-Path $x86 'include') -Filter '*.h' | Copy-Item -Destination (Join-Path $root 'include')
Copy-Item -LiteralPath (Join-Path $x86 'lib\rime.lib') -Destination (Join-Path $root 'lib\rime.lib')
Copy-Item -LiteralPath (Join-Path $x64 'lib\rime.lib') -Destination (Join-Path $root 'lib64\rime.lib')
Write-Output 'Prepared v143 properties and matching x86/x64 import libraries.'
