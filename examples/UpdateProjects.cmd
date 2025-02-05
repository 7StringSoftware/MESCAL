@echo off
@set /p MESCAL_EXAMPLE_VERSION="Enter version: "
@echo on

@set projucer=c:\JUCE\extras\Projucer\Builds\VisualStudio2022\x64\Release\App\Projucer.exe

for /R %%f in (*.jucer) do %projucer% --set-version %MESCAL_EXAMPLE_VERSION% "%%f"
