@echo off&&color 1f&chcp 437 >nul&graftabl 936 >nul
:: code ���Խ� Ӳ�����  @bbs.verybat.cn
:: sc config  winmgmt start= auto >nul 2<&1
:: net start winmgmt 2>nul
set "str=����: BIOS: CPU: ��ʾ��: Ӳ��: ������Ϣ: ����: ��ӡ��: ����: �ڴ�: �Կ�:"
set "code=abcdefghijklmnopqrstuvwxyz"&title %str%

:start
rem cls&setlocal ENABLEDELAYEDEXPANSION
setlocal ENABLEDELAYEDEXPANSION
set "var=!str::= !"&set /a mm=96
for %%a in (!var!) do (
   set /a mm+=1
   set ".!mm!=%%a"
   echo  !code:~0,1!.%%a&echo.
   set "code=!code:~1!"
)
echo  ��������Ӧ�����ѡ�����ѯ�����ݡ��� 1 ȫ����ѯ���� 2 �˳�
echo exit|cmd/kprompt $_e 100 CD 16 B4 4C CD 21$_g$_|debug>nul
set "xuanz=%errorlevel%"&set "exit="&cls
if !xuanz! equ 49 goto ����
if !xuanz! equ 50 pause
set "exit=endlocal&echo.&pause&goto start"
if not defined .!xuanz! endlocal&goto start
goto !.%xuanz%!

:����
cls&echo ����:
for /f "tokens=1* delims==" %%a in (
'wmic BASEBOARD get Manufacturer^,Product^,Version^,SerialNumber /value'
) do (
     set /a tee+=1
     if "!tee!" == "3" echo       ������   = %%b
     if "!tee!" == "4" echo       ��  ��   = %%b
     if "!tee!" == "5" echo       ���к�   = %%b
     if "!tee!" == "6" echo       ��  ��   = %%b
)
%exit%

:BIOS
set /a tee=0&echo.&echo BIOS:
for /f "tokens=1* delims==" %%a in (
'wmic bios  get  CurrentLanguage^,Manufacturer^,SMBIOSBIOSVersion^,SMBIOSMajorVersion^,SMBIOSMinorVersion^,ReleaseDate /value'
) do (
     set /a tee+=1
     if "!tee!" == "3" echo       ��ǰ���� = %%b
     if "!tee!" == "4" echo       �� �� �� = %%b
     if "!tee!" == "5" echo       �������� = %%b
     if "!tee!" == "6" echo       ��    �� = %%b
     if "!tee!" == "7" echo       SMBIOSMajorVersion = %%b
     if "!tee!" == "8" echo       SMBIOSMinorVersion = %%b 
)
%exit%

:CPU
set /a tee=0&echo.&echo CPU:
for /f "tokens=1* delims==" %%a in (
'wmic cpu get name^,ExtClock^,CpuStatus^,Description /value'
) do (
     set /a tee+=1
     if "!tee!" == "3" echo       CPU  �� ��   = %%b
     if "!tee!" == "4" echo       �������汾   = %%b
     if "!tee!" == "5" echo       ��      Ƶ   = %%b
     if "!tee!" == "6" echo       ���Ƽ���Ƶ�� = %%b
)
%exit%

:��ʾ��
set /a tee=0&echo.&echo ��ʾ��:
for /f "tokens=1* delims==" %%a in (
'wmic DESKTOPMONITOR  get name^,ScreenWidth^,ScreenHeight^,PNPDeviceID /value'
) do (
     set /a tee+=1
     if "!tee!" == "3" echo       ��    ��  = %%b
     if "!tee!" == "4" echo       ������Ϣ  = %%b
     if "!tee!" == "5" echo       �� Ļ ��  = %%b
     if "!tee!" == "6" echo       �� Ļ ��  = %%b
)
%exit%

:Ӳ��
set /a tee=0&echo.&echo Ӳ  ��:
for /f "tokens=1* delims==" %%a in (
'wmic DISKDRIVE get model^,interfacetype^,size^,totalsectors^,partitions /value'
) do (
     set /a tee+=1
     if "!tee!" == "3" echo       �ӿ�����  = %%b
     if "!tee!" == "4" echo       Ӳ���ͺ�  = %%b
     if "!tee!" == "5" echo       �� �� ��  = %%b
     if "!tee!" == "6" echo       ��    ��  = %%b
     if "!tee!" == "7" echo       �� �� ��  = %%b
)
%exit%

:������Ϣ
echo.&echo ������Ϣ:
for /f "delims=" %%a in (
'wmic LOGICALDISK where "mediatype='12'" get description^,deviceid^,filesystem^,size^,freespace'
) do echo       %%a
%exit%

:����
set /a tee=0&echo.&echo ��  ��:
for /f "tokens=1* delims==" %%a in (
'wmic NICCONFIG where "index='1'" get ipaddress^,macaddress^,description /value'
) do (
     set /a tee+=1
     if "!tee!" == "3" echo       ��������  = %%b
     if "!tee!" == "4" echo       ����  IP  = %%b
     if "!tee!" == "5" echo       ���� MAC  = %%b
)
%exit%

:��ӡ��
set /a tee=0&echo.&echo ��ӡ��:
for /f "tokens=1* delims==" %%a in ('wmic PRINTER get caption /value') do (
     set /a tee+=1
     if "!tee!" == "3" echo       ��ӡ������  = %%b
)
%exit%

:����
set /a tee=0&echo.&echo �� ��:
for /f "tokens=1* delims==" %%a in ('wmic SOUNDDEV get name^,deviceid /value') do (
     set /a tee+=1
     if "!tee!" == "3" echo       ������Ϣ  = %%b
     if "!tee!" == "4" echo       ��    ��  = %%b
)
%exit%

:�ڴ�
set /a tee=0&echo.&echo ��  ��: 
for /f "delims=" %%i in ('systeminfo 2^>nul^|findstr "�ڴ�"')do echo       %%i
%exit%

:�Կ�
echo.&echo ��  ��:
for /l %%a in (1 1 20) do set tg=    !tg!&set "ko=    !ko!"
set /p=      ������Ҫ 30 ������ ......%tg%<nul
del /f "%TEMP%\temp.txt" 2>nul
dxdiag /t %TEMP%\temp.txt
:�Կ�2  ������Ҫ30������!
if EXIST "%TEMP%\temp.txt" (
set /p=!ko!!tg!<nul
for /f "tokens=1,2,* delims=:" %%a in (
'findstr /c:" Card name:" /c:"Display Memory:" /c:"Current Mode:" "%TEMP%\temp.txt"'
) do (
        set /a tee+=1
        if !tee! == 1 echo       �Կ��ͺ�: %%b
        if !tee! == 2 echo       �Դ��С: %%b
        if !tee! == 3 echo       ��ǰ����: %%b
      )) else (
        ping /n 2 127.1>nul
        goto �Կ�2
)
if !xuanz! equ 49 set "exit=pause"
echo.
%exit%
del /f "%TEMP%\temp.txt" 2>nul
