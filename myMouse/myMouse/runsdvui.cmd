cd /d "C:\myMouse\myMouse" & msbuild "myMouse.vcxproj" /t:sdv /p:inputs="/devenv /clean" /p:configuration="Release" /p:platform="x64" 
exit %errorlevel% 