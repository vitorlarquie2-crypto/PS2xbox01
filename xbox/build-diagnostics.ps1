[CmdletBinding()]
param([string]$OutputDirectory = (Join-Path $PSScriptRoot '..\artifacts\xbox'))
$ErrorActionPreference = 'Stop'
$OutputDirectory = [IO.Path]::GetFullPath($OutputDirectory)
if (Test-Path $OutputDirectory) { throw 'Use a new output directory to avoid mixing packages.' }
$project = Join-Path $PSScriptRoot 'Diagnostics\Diagnostics.csproj'
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$msbuild = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
if (!$msbuild) { throw 'MSBuild with the UWP workload is required.' }
New-Item -ItemType Directory -Path $OutputDirectory | Out-Null
$certificate = New-SelfSignedCertificate -Type Custom -Subject 'CN=PS2XboxDevelopment' `
    -KeyUsage DigitalSignature -FriendlyName 'PS2Xbox development test' `
    -CertStoreLocation 'Cert:\CurrentUser\My' -HashAlgorithm SHA256 `
    -KeyAlgorithm RSA -KeyLength 2048 -NotAfter (Get-Date).AddMonths(12) `
    -TextExtension @('2.5.29.37={text}1.3.6.1.5.5.7.3.3', '2.5.29.19={text}')
try {
    Export-Certificate -Cert $certificate -FilePath (Join-Path $OutputDirectory 'PS2XboxDevelopment.cer') | Out-Null
    & $msbuild $project /restore /t:Build /p:Configuration=Release /p:Platform=x64 `
        /p:GenerateAppxPackageOnBuild=true /p:UapAppxPackageBuildMode=SideloadOnly `
        /p:AppxBundle=Never /p:AppxPackageSigningEnabled=true `
        "/p:PackageCertificateThumbprint=$($certificate.Thumbprint)" "/p:AppxPackageDir=$OutputDirectory\"
    if ($LASTEXITCODE -ne 0) { throw "MSBuild failed: $LASTEXITCODE" }
    $package = Get-ChildItem $OutputDirectory -Recurse -Filter '*.appx' |
        Where-Object { $_.DirectoryName -notmatch '[\\/]Dependencies([\\/]|$)' } | Select-Object -First 1
    if (!$package) { throw 'Build did not produce an APPX.' }
    # Check signer identity; the self-signed chain still requires console acceptance.
    $signature = Get-AuthenticodeSignature $package.FullName
    if (!$signature.SignerCertificate -or $signature.SignerCertificate.Thumbprint -ne $certificate.Thumbprint) {
        throw 'The APPX was not signed with the expected certificate.'
    }
    Get-ChildItem $OutputDirectory -Recurse -File | Get-FileHash -Algorithm SHA256 |
        ForEach-Object { '{0}  {1}' -f $_.Hash, $_.Path.Substring($OutputDirectory.Length + 1) } |
        Set-Content (Join-Path $OutputDirectory 'SHA256SUMS.txt')
} finally {
    Remove-Item "Cert:\CurrentUser\My\$($certificate.Thumbprint)" -ErrorAction SilentlyContinue
}
