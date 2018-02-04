if (Test-Path .\build\breakout.exe)
{
    .\build\breakout.exe $args 
}
else
{
    Write-Output(".\build\breakout.exe does not exist!")
}

