@REM @ECHO OFF
@REM setlocal enabledelayedexpansion
@REM @REM setlocal enableextensions disabledelayedexpansion
@REM cls
@REM if exist "Build\" (
@REM     @REM echo created Build\
@REM ) else (
@REM     MD Build\
@REM )
@REM if exist "Build\asm" (
@REM     @REM echo "!FOLDER_PATH!"
@REM ) else (
@REM     MD Build\asm\
@REM )
@REM if exist "Build\obj" (
@REM     @REM echo "!FOLDER_PATH!"
@REM ) else (
@REM     MD Build\obj\
@REM )
@REM if exist "Build\Output" (
@REM     @REM echo "!FOLDER_PATH!"
@REM ) else (
@REM     MD Build\Output\
@REM )
@REM cd Build/Output
@REM for /r %%a in (*) do (
@REM     if "%%~nxa"=="Compiler_inputFile.txt" (
@REM     Del "Compiler_inputFile.txt"
@REM     )
@REM     if "%%~xa"==".hex" (
@REM         Del %%~nxa
@REM         )
@REM     if "%%~xa"==".map" (
@REM         Del %%~nxa
@REM         )
@REM     if "%%~xa"==".abs" (
@REM         Del %%~nxa
@REM         )
@REM     if "%%~xa"==".sni" (
@REM         Del %%~nxa
@REM         )
@REM     if "%%~xa"==".xml" (
@REM         Del %%~nxa
@REM         )
@REM )

@REM cd ..
@REM cd ..
@REM echo -c>> Build/Output/Compiler_inputFile.txt
@REM @REM echo %cd%



@REM set "var=%cd%"
@REM cd ..
@REM set "var1=%cd%"
@REM cd %var%
@REM for /F %%a in ("!var1!") do set "Project_Name=!var:%%a=!"
@REM set Project_Name=%Project_Name:*\=%
@REM @REM echo %Project_Name%
@REM ECHO ------ Sloki Software Technologies------
@REM ECHO ------ Start build(%Project_Name%) ------
@REM SET FOLDER_PATH=%CD%
@REM SET FOLDER_C_PATH=%CD..%
@REM SET FOLDER_H_PATH=%CD..%
@REM SET FOLDER_ASM_PATH=%CD..%
@REM for /R  %%f in (*) do (
@REM     for %%I in ("%%~dp$PATH:f.") do (
@REM         SET FOLDER_PATH=%%~fI
@REM         @REM echo "!FOLDER_PATH!"
@REM     )
@REM     if "%%~xf"==".h" (
@REM         if "!FOLDER_H_PATH!" EQU  "!FOLDER_PATH!" (
@REM         @REM echo "%%~dp$PATH:f"
@REM         ) else (
@REM             for %%K in ("%%~dp$PATH:f.") do (
@REM                 SET FOLDER_H_PATH=%%~fK
@REM                 @REM echo "!FOLDER_H_PATH!"

@REM                 if "%CD%" EQU  "%%~fK" (
@REM                     @REM Set p=%%~fK
@REM                     Echo -I.>> Build/Output/Compiler_inputFile.txt
@REM                 ) else (
@REM                     Set p=%%~fK
@REM                     Echo -I!p:%__CD__%=!>> Build/Output/Compiler_inputFile.txt 
@REM                 )

@REM                 @REM echo -I%%~fK>> Build/Output/Compiler_inputFile.txt
@REM             )
@REM         )
@REM     )
@REM     if "%%~xf"==".c" (
@REM         if "!FOLDER_C_PATH!" EQU  "!FOLDER_PATH!" (
@REM         @REM echo "%%~dp$PATH:f"
@REM         ) else (
@REM             for %%K in ("%%~dp$PATH:f.") do (
@REM                 SET FOLDER_C_PATH=%%~fK
@REM                 @REM echo "!FOLDER_C_PATH!"

@REM                 if "%CD%" EQU  "%%~fK" (
@REM                     @REM Set p=%%~fK
@REM                     Echo *.c>> Build/Output/Compiler_inputFile.txt
@REM                 ) else (
@REM                     Set p=%%~fK
@REM                     Echo !p:%__CD__%=!\*.c>> Build/Output/Compiler_inputFile.txt 
@REM                 )
@REM                 @REM Set p=%%~fK
@REM                 @REM echo %%~fK
@REM                 @REM Echo !p:%__CD__%=!\*.c>> Build/Output/Compiler_inputFile.txt

@REM                 @REM echo %%~fK\*.c>> Build/Output/Compiler_inputFile.txt
@REM                 @REM Set p=%%~fK
@REM                 @REM Echo ^>!p:%__CD__%=!
@REM             )
@REM         )
@REM         Set p=%%~f
@REM         Echo ^>!p:%__CD__%=!
@REM     )
@REM     if "%%~xf"==".asm" (
@REM         if "!FOLDER_ASM_PATH!" EQU  "!FOLDER_PATH!" (
@REM         @REM echo "%%~dp$PATH:f"
@REM         ) else (
@REM             for %%K in ("%%~dp$PATH:f.") do (
@REM                 SET FOLDER_ASM_PATH=%%~fK
@REM                 @REM echo "!FOLDER_ASM_PATH!"

@REM                 if "%CD%" EQU  "%%~fK" (
@REM                     @REM Set p=%%~fK
@REM                     Echo *.asm>> Build/Output/Compiler_inputFile.txt
@REM                 ) else (
@REM                     Set p=%%~fK
@REM                     Echo !p:%__CD__%=!\*.asm>> Build/Output/Compiler_inputFile.txt 
@REM                 )
@REM                 @REM Set p=%%~fK
@REM                 @REM Echo !p:%__CD__%=!\*.asm>> Build/Output/Compiler_inputFile.txt

@REM                 @REM echo %%~fK\*.asm>> Build/Output/Compiler_inputFile.txt
@REM             )
@REM         )
@REM         Set p=%%f
@REM         Echo ^>!p:%__CD__%=!
@REM     )
@REM )
@REM @REM ECHO ------ Start build(VCU_ASW_V1) ------
@REM @REM SET FOLDER_PATH=%CD%
@REM @REM SET FOLDER1_PATH=%CD..%
@REM @REM for /R  %%f in (*.asm) do (
@REM @REM     for %%I in ("%%~dp$PATH:f.") do (
@REM @REM         SET FOLDER_PATH=%%~fI
@REM @REM         @REM echo "!FOLDER_PATH!"
@REM @REM     )
@REM @REM     if "!FOLDER1_PATH!" EQU  "!FOLDER_PATH!" (
@REM @REM         @REM echo "%%~dp$PATH:f"
@REM @REM     ) else (
@REM @REM             for %%K in ("%%~dp$PATH:f.") do (
@REM @REM         SET FOLDER1_PATH=%%~fK
@REM @REM         @REM echo "!FOLDER1_PATH!"
@REM @REM         echo %%~fK\*.asm>> Build/Output/Compiler_inputFile.txt
@REM @REM         @REM echo %%~fK\*.asm
@REM @REM         )
@REM @REM     )
@REM @REM     Set p=%%f
@REM @REM     Echo ^>!p:%__CD__%=!
@REM @REM )
@REM @REM SET FOLDER_PATH=%CD%
@REM @REM SET FOLDER1_PATH=%CD..%
@REM @REM for /R  %%f in (*.c) do (
@REM @REM     for %%I in ("%%~dp$PATH:f.") do (
@REM @REM         SET FOLDER_PATH=%%~fI
@REM @REM         @REM echo "!FOLDER_PATH!"
@REM @REM     )
@REM @REM     if "!FOLDER1_PATH!" EQU  "!FOLDER_PATH!" (
@REM @REM         echo "%%~dp$PATH:f"
@REM @REM     ) else (
@REM @REM             for %%K in ("%%~dp$PATH:f.") do (
@REM @REM         SET FOLDER1_PATH=%%~fK
@REM @REM         @REM echo "!FOLDER1_PATH!"
@REM @REM         echo %%~fK\*.c>> Build/Output/Compiler_inputFile.txt
@REM @REM         @REM echo %%~fK\*.c
@REM @REM         )
@REM @REM     )
@REM @REM     Set p=%%f
@REM @REM     Echo ^>!p:%__CD__%=!
@REM @REM )
@REM @REM SET FOLDER_PATH=%CD%
@REM @REM SET FOLDER1_PATH=%CD..%
@REM @REM for /R  %%f in (*.h) do (
@REM @REM     for %%I in ("%%~dp$PATH:f.") do (
@REM @REM         SET FOLDER_PATH=%%~fI
@REM @REM         @REM echo "!FOLDER_PATH!"
@REM @REM     )
@REM @REM     if "!FOLDER1_PATH!" EQU  "!FOLDER_PATH!" (
@REM @REM         @REM echo "%%~dp$PATH:f"
@REM @REM     ) else (
@REM @REM             for %%K in ("%%~dp$PATH:f.") do (
@REM @REM         SET FOLDER1_PATH=%%~fK
@REM @REM         @REM echo "!FOLDER1_PATH!"
@REM @REM         echo -I%%~fK>> Build/Output/Compiler_inputFile.txt
@REM @REM         @REM echo -I%%~fK
@REM @REM         )
@REM @REM     )
@REM @REM ) 
@REM echo -Xobj_path=.\Build\obj>> Build/Output/Compiler_inputFile.txt
@REM echo -Xasm_option=-Xprn_path=.\Build\asm>> Build/Output/Compiler_inputFile.txt
@REM echo -Xcommon=rh850>> Build/Output/Compiler_inputFile.txt
@REM SET COMPILER_PATH=Setting\COMPILER\V2.03.00\bin
@REM cd Build/asm
@REM for %%i in ("*.prn") do (
@REM     Del "*.prn"
@REM )
@REM cd ..
@REM cd obj
@REM for %%i in ("*.obj") do (
@REM     Del "*.obj"
@REM )
@REM cd ..
@REM cd ..
@REM %COMPILER_PATH%\ccrh @./Build/Output/Compiler_inputFile.txt
@REM @REM cls
@REM cd Build/Output
@REM for /r %%a in (*) do (
@REM     if "%%~nxa"=="Compiler_inputFile.txt" (
@REM     Del "Compiler_inputFile.txt"
@REM     )
@REM )
@REM cd ..
@REM cd obj
@REM for %%y in ("*.obj") do (
@REM     cd ..
@REM     cd Output
@REM     echo -input=Build/obj/%%y>> Compiler_inputFile.txt
@REM     cd ..
@REM     cd obj
@REM )
@REM cd ..
@REM cd ..
@REM cd Lib
@REM for /r %%a in (*) do (
@REM     if "%%~xa"==".lib" (
@REM         cd ..
@REM         echo -library=%%a>> Build/Output/Compiler_inputFile.txt
@REM         cd Lib
@REM         @REM echo %%a
@REM     ) 
@REM )

@REM @REM for /r %%a in (*) do (
@REM @REM     if "%%~xa"==".lib" (
@REM @REM         cd ..
@REM @REM         @REM for %%K in ("%%~dp$PATH:a.") do (
@REM @REM         @REM     Set p=%%~aK
@REM @REM         @REM     Echo -library=!p:%__CD__%=!>> Build/Output/Compiler_inputFile.txt 
@REM @REM         @REM     Echo -library=!p:%__CD__%=!
@REM @REM         @REM )
@REM @REM         echo -library=%%a>> Build/Output/Compiler_inputFile.txt
@REM @REM         @REM echo -library=%%a
@REM @REM         cd Lib
@REM @REM     ) 
@REM @REM )
@REM cd ..
@REM @REM echo -Xno_warning=61017>> Build/Output/Compiler_inputFile.txt
@REM echo -nocompress>> Build/Output/Compiler_inputFile.txt
@REM echo -NOOPtimize>> Build/Output/Compiler_inputFile.txt
@REM echo -output=Build/Output/%Project_Name%.abs>> Build/Output/Compiler_inputFile.txt
@REM echo -list=Build/Output/%Project_Name%.map>> Build/Output/Compiler_inputFile.txt
@REM echo -library=Setting/COMPILER/V2.03.00/lib/v850e3v5/rhf4n.lib>> Build/Output/Compiler_inputFile.txt
@REM echo -library=Setting/COMPILER/V2.03.00/lib/v850e3v5/libmalloc.lib>> Build/Output/Compiler_inputFile.txt
@REM setlocal enableextensions disabledelayedexpansion
@REM set "var="
@REM for /f "usebackq delims=" %%a in ("Setting/Linker_File.txt") do (
@REM     setlocal enabledelayedexpansion   
@REM     for /f "tokens=* delims=¬" %%b in ("¬!var!") do endlocal & set "var=%%b%%a"
@REM )
@REM @REM echo %var%
@REM echo -start=%var%>> Build/Output/Compiler_inputFile.txt
@REM @REM echo -start=RESET/e000,EIINTTBL.const/0000E400,.text,.const,.INIT_DSEC.const,.INIT_BSEC.const,.data,R_FDL_Text.text,R_FDL_Const.const,R_FCL_CODE_ROM.text,R_FCL_CONST.const,R_FCL_CODE_USRINT.text,R_FCL_CODE_USR.text,R_FCL_CODE_RAM.text,R_FCL_CODE_ROMRAM.text,R_FCL_CODE_RAM_EX_PROT.text/0000FF70,FCL_RESERVED.bss/FEBE0000,.data.R,.bss,R_FCL_DATA.bss,.stack.bss,R_FDL_Data.bss,R_FDL_CodeRam.bss/FEDE8000>> Build/Output/Compiler_inputFile.txt
@REM @REM echo -start=RESET/0,EIINTTBL.const/00000200,.const,.INIT_DSEC.const,.INIT_BSEC.const,.text,.data,R_FDL_Text.text,R_FDL_Const.const/00008000,.data.R,.bss,.stack.bss,R_FDL_Data.bss,R_FDL_CodeRam.bss/FEDE8000>> Build/Output/Compiler_inputFile.txt
@REM echo -rom=.data=.data.R>> Build/Output/Compiler_inputFile.txt
@REM echo -stack>> Build/Output/Compiler_inputFile.txt
@REM echo -total_size>> Build/Output/Compiler_inputFile.txt
@REM echo -nologo>> Build/Output/Compiler_inputFile.txt
@REM echo -output=Build/Output/%Project_Name%.hex>> Build/Output/Compiler_inputFile.txt
@REM @REM echo -byte_count=1>> Build/Output/Compiler_inputFile.txt
@REM echo -form=hexadecimal>> Build/Output/Compiler_inputFile.txt
@REM @REM echo -FIX_RECORD_LENGTH_AND_ALIGN=10>> Build/Output/Compiler_inputFile.txt
@REM echo byte_count=10>> Build/Output/Compiler_inputFile.txt
@REM echo -exit>> Build/Output/Compiler_inputFile.txt
@REM %COMPILER_PATH%\rlink -subcommand=./Build/Output/Compiler_inputFile.txt
@REM cd Build/Output
@REM for /r %%a in (*) do (
@REM     if "%%~nxa"=="Compiler_inputFile.txt" (
@REM     Del "Compiler_inputFile.txt"
@REM     )
@REM )
@REM cd ..
@REM @REM to keep obj file .prn file comment below line till END... is thire 
@REM cd asm
@REM for %%i in ("*.prn") do (
@REM     Del "*.prn"
@REM )
@REM cd ..
@REM cd obj
@REM for %%i in ("*.obj") do (
@REM     Del "*.obj"
@REM )
@REM cd ..
@REM @REM PAUSE
@REM cd Output
@REM for %%i in ("*.abs") do (
@REM     Del "*.abs"
@REM )
@REM for %%i in ("*.map") do (
@REM     Del "*.map"
@REM )
@REM for %%i in ("*.sni") do (
@REM     Del "*.sni"
@REM )

@REM @REM echo ^<?xml version="1.0" encoding="UTF-8"?^> >sFlasher_config.xml
@REM @REM echo ^<ECU_NAME^> >>sFlasher_config.xml
@REM @REM echo   ^<PROGRAMMER^>VCU^</PROGRAMMER^> >>sFlasher_config.xml
@REM @REM echo   ^<TOTAL_BLOCKS^>3^</TOTAL_BLOCKS^> >>sFlasher_config.xml
@REM @REM echo   ^<DEFAULT_BAUDRATE^>500^</DEFAULT_BAUDRATE^> >>sFlasher_config.xml
@REM @REM     echo   ^<MEM_BLOCKS^> >>sFlasher_config.xml
@REM @REM     echo        ^<LOGICAL_BLOCK name="ASW"^> >>sFlasher_config.xml
@REM @REM     echo            ^<PHY_BLOCK^> >>sFlasher_config.xml
@REM @REM 	echo	            ^<BLOCK_NAME^>ASW1^</BLOCK_NAME^> >>sFlasher_config.xml
@REM @REM 	echo	            ^<BLOCK_ID^>0xB101^</BLOCK_ID^> >>sFlasher_config.xml
@REM @REM 	echo	            ^<BLOCK_STARTING_ADDRESS^>0x0000E000^</BLOCK_STARTING_ADDRESS^> >>sFlasher_config.xml
@REM @REM 	echo	            ^<BLOCK_SIZE^>0x00002000^</BLOCK_SIZE^> >>sFlasher_config.xml
@REM @REM     echo            ^</PHY_BLOCK^> >>sFlasher_config.xml
@REM @REM     echo        ^</LOGICAL_BLOCK^> >>sFlasher_config.xml
@REM @REM     echo   ^</MEM_BLOCKS^> >>sFlasher_config.xml
@REM @REM for /f "tokens=*" %%a in (%Project_Name%.hex) do (
@REM @REM   if "%%a" == ":020000021000EC" (
@REM @REM     echo   ^<MEM_BLOCKS^> >>sFlasher_config.xml
@REM @REM     echo        ^<LOGICAL_BLOCK name="ASW"^> >>sFlasher_config.xml
@REM @REM     echo            ^<PHY_BLOCK^> >>sFlasher_config.xml
@REM @REM 	echo	            ^<BLOCK_NAME^>ASW2^</BLOCK_NAME^> >>sFlasher_config.xml
@REM @REM 	echo	            ^<BLOCK_ID^>0xB102^</BLOCK_ID^> >>sFlasher_config.xml
@REM @REM 	echo	            ^<BLOCK_STARTING_ADDRESS^>0x00010000^</BLOCK_STARTING_ADDRESS^> >>sFlasher_config.xml
@REM @REM 	echo	            ^<BLOCK_SIZE^>0x00010000^</BLOCK_SIZE^> >>sFlasher_config.xml
@REM @REM     echo            ^</PHY_BLOCK^> >>sFlasher_config.xml
@REM @REM     echo        ^</LOGICAL_BLOCK^> >>sFlasher_config.xml
@REM @REM     echo   ^</MEM_BLOCKS^> >>sFlasher_config.xml
@REM @REM   )
@REM @REM   if "%%a" == ":020000022000DC" (

@REM @REM     echo   ^<MEM_BLOCKS^> >>sFlasher_config.xml
@REM @REM     echo        ^<LOGICAL_BLOCK name="ASW"^> >>sFlasher_config.xml
@REM @REM     echo            ^<PHY_BLOCK^> >>sFlasher_config.xml
@REM @REM 	echo	            ^<BLOCK_NAME^>ASW3^</BLOCK_NAME^> >>sFlasher_config.xml
@REM @REM 	echo	            ^<BLOCK_ID^>0xB103^</BLOCK_ID^> >>sFlasher_config.xml
@REM @REM 	echo	            ^<BLOCK_STARTING_ADDRESS^>0x00020000^</BLOCK_STARTING_ADDRESS^> >>sFlasher_config.xml
@REM @REM 	echo	            ^<BLOCK_SIZE^>0x00010000^</BLOCK_SIZE^> >>sFlasher_config.xml
@REM @REM     echo            ^</PHY_BLOCK^> >>sFlasher_config.xml
@REM @REM     echo        ^</LOGICAL_BLOCK^> >>sFlasher_config.xml
@REM @REM     echo   ^</MEM_BLOCKS^> >>sFlasher_config.xml
@REM @REM   )
@REM @REM )
@REM @REM echo ^</ECU_NAME^> >>sFlasher_config.xml
@REM cd ..
@REM PAUSE
@REM @REM END...
@REM @REM exit /b

















@REM @REM for %%y in ("*.obj") do (
@REM     @REM if "%%~nxy" NEQ  "r_fdl_user_if_init.obj" (
@REM     @REM     if "%%~nxy" NEQ  "r_fdl_user_if.obj" (
@REM     @REM         if "%%~nxy" NEQ  "r_fdl_hw_access.obj" (
@REM     @REM             if "%%~nxy" NEQ  "fdl_descriptor.obj" (
@REM     @REM                 if "%%~nxy" NEQ  "fdl_user.obj" (
@REM                         @REM echo -input=Build/obj/%%y>> input_objFile.txt
@REM     @REM     )
@REM     @REM     )       
@REM     @REM     )
@REM     @REM     )
@REM     @REM )
@REM )
@REM @REM for %%y in ("*.obj") do (
@REM @REM     if "%%~nxy" EQU "r_fdl_user_if_init.obj" (
@REM @REM         echo -input=Build/obj/%%y>> input_objFile.txt
@REM @REM     ) else if "%%~nxy" EQU "r_fdl_user_if.obj" (
@REM @REM         echo -input=Build/obj/%%y>> input_objFile.txt
@REM @REM     ) else if "%%~nxy" EQU "r_fdl_hw_access.obj" (
@REM @REM         echo -input=Build/obj/%%y>> input_objFile.txt
@REM @REM     ) else if "%%~nxy" EQU "fdl_descriptor.obj" (
@REM @REM         echo -input=Build/obj/%%y>> input_objFile.txt
@REM @REM     ) else if "%%~nxy" EQU "fdl_user.obj" (
@REM @REM         echo -input=Build/obj/%%y>> input_objFile.txt
@REM @REM     )
@REM @REM )    



@ECHO OFF
setlocal enabledelayedexpansion
cls

REM =========================
REM Create folders
REM =========================
if not exist "Build\"        MD "Build\"
if not exist "Build\asm\"    MD "Build\asm\"
if not exist "Build\obj\"    MD "Build\obj\"
if not exist "Build\Output\" MD "Build\Output\"

REM =========================
REM Clean old outputs
REM =========================
pushd "Build\Output"
for /r %%a in (*) do (
    if /I "%%~nxa"=="Compiler_inputFile.txt" del /q "Compiler_inputFile.txt"
    if /I "%%~xa"==".hex" del /q "%%~nxa"
    if /I "%%~xa"==".map" del /q "%%~nxa"
    if /I "%%~xa"==".abs" del /q "%%~nxa"
    if /I "%%~xa"==".sni" del /q "%%~nxa"
    if /I "%%~xa"==".xml" del /q "%%~nxa"
)
popd

REM =========================
REM Start creating compiler input
REM =========================
echo -c>> Build\Output\Compiler_inputFile.txt

REM Project name extraction (your original method)
set "var=%cd%"
cd ..
set "var1=%cd%"
cd %var%
for /F %%a in ("!var1!") do set "Project_Name=!var:%%a=!"
set "Project_Name=%Project_Name:*\=%"

ECHO ------ Sloki Software Technologies------
ECHO ------ Start build(%Project_Name%) ------
SET FOLDER_PATH=%CD%
SET FOLDER_C_PATH=%CD..%
SET FOLDER_H_PATH=%CD..%
SET FOLDER_ASM_PATH=%CD..%

REM =========================
REM Scan project: add include paths + source globs
REM =========================
for /R %%f in (*) do (
    for %%I in ("%%~dp$PATH:f.") do (
        SET FOLDER_PATH=%%~fI
    )

    if /I "%%~xf"==".h" (
        if "!FOLDER_H_PATH!" NEQ "!FOLDER_PATH!" (
            for %%K in ("%%~dp$PATH:f.") do (
                SET FOLDER_H_PATH=%%~fK

                if "%CD%" EQU "%%~fK" (
                    Echo -I.>> Build\Output\Compiler_inputFile.txt
                ) else (
                    Set "p=%%~fK"
                    Echo -I!p:%__CD__%=!>> Build\Output\Compiler_inputFile.txt
                )
            )
        )
    )

    if /I "%%~xf"==".c" (
        if "!FOLDER_C_PATH!" NEQ "!FOLDER_PATH!" (
            for %%K in ("%%~dp$PATH:f.") do (
                SET FOLDER_C_PATH=%%~fK

                if "%CD%" EQU "%%~fK" (
                    Echo *.c>> Build\Output\Compiler_inputFile.txt
                ) else (
                    Set "p=%%~fK"
                    Echo !p:%__CD__%=!\*.c>> Build\Output\Compiler_inputFile.txt
                )
            )
        )
    )

    if /I "%%~xf"==".asm" (
        if "!FOLDER_ASM_PATH!" NEQ "!FOLDER_PATH!" (
            for %%K in ("%%~dp$PATH:f.") do (
                SET FOLDER_ASM_PATH=%%~fK

                if "%CD%" EQU "%%~fK" (
                    Echo *.asm>> Build\Output\Compiler_inputFile.txt
                ) else (
                    Set "p=%%~fK"
                    Echo !p:%__CD__%=!\*.asm>> Build\Output\Compiler_inputFile.txt
                )
            )
        )
    )
)

echo -Xobj_path=.\Build\obj>> Build\Output\Compiler_inputFile.txt
echo -Xasm_option=-Xprn_path=.\Build\asm>> Build\Output\Compiler_inputFile.txt
echo -Xcommon=rh850>> Build\Output\Compiler_inputFile.txt

SET "COMPILER_PATH=Setting\COMPILER\V2.03.00\bin"

REM =========================
REM Clean intermediate files
REM =========================
if exist "Build\asm\*.prn" del /q "Build\asm\*.prn"
if exist "Build\obj\*.obj" del /q "Build\obj\*.obj"

REM =========================
REM Compile
REM =========================
%COMPILER_PATH%\ccrh @.\Build\Output\Compiler_inputFile.txt
if errorlevel 1 goto :FAIL

REM Remove compiler cmd file
if exist "Build\Output\Compiler_inputFile.txt" del /q "Build\Output\Compiler_inputFile.txt"

REM =========================
REM Create linker subcommand file
REM =========================
pushd "Build\obj"
for %%y in ("*.obj") do (
    popd
    echo -input=Build/obj/%%y>> Build\Output\Compiler_inputFile.txt
    pushd "Build\obj"
)
popd

REM Add libs found in Lib folder
if exist "Lib\" (
    pushd "Lib"
    for /r %%a in (*.lib) do (
        popd
        echo -library=%%a>> Build\Output\Compiler_inputFile.txt
        pushd "Lib"
    )
    popd
)

REM Linker settings
echo -nocompress>> Build\Output\Compiler_inputFile.txt
echo -NOOPtimize>> Build\Output\Compiler_inputFile.txt
echo -output=Build/Output/%Project_Name%.abs>> Build\Output\Compiler_inputFile.txt
echo -list=Build/Output/%Project_Name%.map>> Build\Output\Compiler_inputFile.txt
echo -library=Setting/COMPILER/V2.03.00/lib/v850e3v5/rhf4n.lib>> Build\Output\Compiler_inputFile.txt
echo -library=Setting/COMPILER/V2.03.00/lib/v850e3v5/libmalloc.lib>> Build\Output\Compiler_inputFile.txt

REM Read start/section mapping from file
setlocal enableextensions disabledelayedexpansion
set "var="
for /f "usebackq delims=" %%a in ("Setting/Linker_File.txt") do (
    setlocal enabledelayedexpansion
    for /f "tokens=* delims=¬" %%b in ("¬!var!") do endlocal & set "var=%%b%%a"
)
endlocal & set "var=%var%"

echo -start=%var%>> Build\Output\Compiler_inputFile.txt
echo -rom=.data=.data.R>> Build\Output\Compiler_inputFile.txt
echo -stack>> Build\Output\Compiler_inputFile.txt
echo -total_size>> Build\Output\Compiler_inputFile.txt
echo -nologo>> Build\Output\Compiler_inputFile.txt
echo -output=Build/Output/%Project_Name%.hex>> Build\Output\Compiler_inputFile.txt
echo -form=hexadecimal>> Build\Output\Compiler_inputFile.txt
echo byte_count=10>> Build\Output\Compiler_inputFile.txt
echo -exit>> Build\Output\Compiler_inputFile.txt

REM =========================
REM Link
REM =========================
%COMPILER_PATH%\rlink -subcommand=.\Build\Output\Compiler_inputFile.txt
if errorlevel 1 goto :FAIL

REM Remove linker cmd file
if exist "Build\Output\Compiler_inputFile.txt" del /q "Build\Output\Compiler_inputFile.txt"

REM Optional cleanup (your original behavior)
if exist "Build\asm\*.prn" del /q "Build\asm\*.prn"
if exist "Build\obj\*.obj" del /q "Build\obj\*.obj"
if exist "Build\Output\*.abs" del /q "Build\Output\*.abs"
if exist "Build\Output\*.map" del /q "Build\Output\*.map"
if exist "Build\Output\*.sni" del /q "Build\Output\*.sni"

REM =========================
REM Final success condition: hex must exist
REM =========================
if exist "Build\Output\%Project_Name%.hex" (
    echo.
    echo BUILD SUCCESSFULL - Please find the HEX file here:
    echo %CD%\Build\Output\%Project_Name%.hex
    echo.
    goto :END
) else (
    goto :FAIL
)

:FAIL
echo.
echo BUILD FAILED.
echo Check compiler/linker errors above.
echo.
goto :END

:END
PAUSE
exit /b
