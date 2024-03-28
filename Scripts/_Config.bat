
set "UE_ENGINE_ROOT_DIRECTORY=E:\UE_5.3.2_Sources\"
set "BUILD_PATH=E:\PantheonMPExperimental\"

set "UE_ENGINE_BUILD_DIRECTORY=%UE_ENGINE_ROOT_DIRECTORY%Engine\Build\"
set "UE_EDITOR_DIRECTORY=%UE_ENGINE_ROOT_DIRECTORY%Engine\Binaries\Win64\"

set PROJECT_NAME=PresenceSystem
set "PROJECT_UPROJ=%PROJECT_NAME%.uproj"

:: clean PROJECT_DIRECTORY path trick.
set PROJECT_DIRECTORY=%~dp0..\
:: cd to proper hard drive.
%~d0
:: cd to proper directory.
cd %PROJECT_DIRECTORY%
set PROJECT_DIRECTORY=%cd%\

set "UE_EDITOR_PATH=%UE_EDITOR_DIRECTORY%UnrealEditor.exe"
set "UE_SERVER_PATH=.\Binaries\Win64\PresenceSystemServer.exe"
set "UE_CLIENT_PATH=.\Binaries\Win64\PresenceSystemClient.exe"
set "PROJECT_PATH=%PROJECT_DIRECTORY%PresenceSystem.uproject"

:: You can edit arguments here (docs : https://docs.unrealengine.com/latest/INT/Programming/Basics/CommandLineArguments/index.html)
set "SERVER_ARGS= -log"
set "CLIENT_ARGS= -log -windowed -resx=1366 -resy=768"

set "COOKED_SERVER_COMMAND=%UE_SERVER_PATH% %PROJECT_PATH% %SERVER_ARGS%"
set "COOKED_CLIENT_COMMAND=%UE_CLIENT_PATH% %PROJECT_PATH% %CLIENT_ARGS%"

set "PROJECT_SERVER_COMMAND=%UE_EDITOR_PATH% %PROJECT_PATH% -server %SERVER_ARGS%"
set "PROJECT_CLIENT_COMMAND=%UE_EDITOR_PATH% %PROJECT_PATH% -game %CLIENT_ARGS%"




