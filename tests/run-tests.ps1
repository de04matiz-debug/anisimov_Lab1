param([string]$MSBuild = 'C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe')
$ErrorActionPreference = 'Stop'
$repo = Split-Path $PSScriptRoot -Parent
& $MSBuild "$repo/anisimov_Lab1.slnx" /p:Configuration=Debug /p:Platform=x64 /v:minimal /nologo
if ($LASTEXITCODE -ne 0) { throw 'Application build failed' }
& $MSBuild "$PSScriptRoot/file-tests.vcxproj" /p:Configuration=Debug /p:Platform=x64 /v:minimal /nologo
if ($LASTEXITCODE -ne 0) { throw 'Test build failed' }
$testDirectory = Join-Path ([IO.Path]::GetTempPath()) ('anisimov-lab1-tests-' + [guid]::NewGuid())
& "$PSScriptRoot/x64/Debug/file-tests.exe" $testDirectory
if ($LASTEXITCODE -ne 0) { throw 'File tests failed' }

function Invoke-ConsoleTest([string]$Name, [string]$InputText, [string[]]$Expected)
{
    $start = [Diagnostics.ProcessStartInfo]::new()
    $start.FileName = "$repo/x64/Debug/anisimov_Lab1.exe"
    $start.WorkingDirectory = $testDirectory
    $start.UseShellExecute = $false
    $start.CreateNoWindow = $true
    $start.RedirectStandardInput = $true
    $start.RedirectStandardOutput = $true
    $process = [Diagnostics.Process]::Start($start)
    $process.StandardInput.Write($InputText)
    $process.StandardInput.Close()
    if (-not $process.WaitForExit(5000)) { $process.Kill(); throw "Timeout: $Name" }
    $output = $process.StandardOutput.ReadToEnd()
    if ($process.ExitCode -ne 0) { throw "Nonzero exit: $Name" }
    foreach ($text in $Expected) {
        if (-not $output.Contains($text)) { throw "Missing '$text' in test '$Name': $output" }
    }
    Write-Host "PASS: $Name"
    $process.Dispose()
}
Invoke-ConsoleTest 'absent objects' "3`n4`n5`n0`n" @('Pipe is not created.', 'Station is not created.', 'First add a pipe.', 'First add a station.')
Invoke-ConsoleTest 'menu validation' "x`n1.5`n3 junk`n99999999999999999999`n8`n0`n" @('Invalid input.', 'Program finished.')
Invoke-ConsoleTest 'names, ranges and repair' "1`n   `nPipe with spaces`n0`n-2`n1e999`n12.5x`n12.5`n700.5`n700`n2`n1`n4`n0`n3`n0`n" @('The name must not be empty.', 'Invalid input.', 'Name: Pipe with spaces', 'Under repair: No')
Invoke-ConsoleTest 'workshop limits' "2`nStation with spaces`n1`n2`n0`n0`n1`n5`n2`n5`n1`n5`n1`n5`n2`n3`n0`n" @('There are no working workshops.', 'Workshop started.', 'All workshops are already working.', 'Workshop stopped.', 'Working workshops: 0')
Invoke-ConsoleTest 'EOF at menu' '' @('Program finished.')
Invoke-ConsoleTest 'EOF during pipe creation' "1`nPartial pipe`n" @('Length (km, > 0):')
Invoke-ConsoleTest 'EOF during station creation' "2`nPartial station`n" @('Number of workshops (> 0):')
Invoke-ConsoleTest 'EOF after invalid input' "junk`n" @('Invalid input.', 'Program finished.')
Write-Host 'PASS: 8 console scenarios. Temporary fixtures:' $testDirectory
