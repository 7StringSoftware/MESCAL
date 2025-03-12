:: fxc.exe needs the path to d2d1effecthelpers.hlsli (/I parameter).
:: Parse the Visual Studio macro into a form usable by fxc.exe.
set INCLUDEPATHS=%1
set INCLUDEPATHS=%INCLUDEPATHS:;=" /I "%
@echo.
@echo step 1
@echo.
fxc /Vi /T lib_4_0_level_9_1_ps_only ../../Source/CustomPixelShader.hlsl /D D2D_FUNCTION /D D2D_ENTRY=main /Fl %2CustomPixelShader.fxlib /nologo /I %INCLUDEPATHS%
@echo.
@echo step 2
@echo.
fxc /T ps_4_0_level_9_1 ../../Source/CustomPixelShader.hlsl /D D2D_FULL_SHADER /D D2D_ENTRY=main /E main /setprivate "%2CustomPixelShader.fxlib" /Fo "%2CustomPixelShader.cso" /Fh %2CustomPixelShader.h /nologo /I %INCLUDEPATHS%
