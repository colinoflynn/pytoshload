ECHO OFF
CLS
CALL PREP_TLCS_870.BAT

ECHO CLEARING...
CALL CLEAN.BAT
ECHO ****************

GOTO CONT

-$   silent
-w0  warning level
-c   create object
-XS  optimize size
-l   create list file
-o   output file name
-O0  no optimization
-XA4 alignment area section 4 bytes
-XS  optimize size

:cont

SET CC_OPT=-$ -DTMP86FM29 -DBUFFER_SIZE=0x40 -DRW_UNALIGNED -w1 -c -l -XS -I FLASH -I SRC -I SYSTEM -I SYSTEM\TLCS_870

ECHO COMPILING...
cc870c %CC_OPT% SYSTEM\TLCS_870\System.c
IF ERRORLEVEL 1 PAUSE
cc870c %CC_OPT% FLASH\FlashEE.c
IF ERRORLEVEL 1 PAUSE
cc870c %CC_OPT% SRC\Main.c
IF ERRORLEVEL 1 PAUSE
cc870c %CC_OPT% SRC\CRC.c
IF ERRORLEVEL 1 PAUSE
ECHO ****************

ECHO LINKING...
TULINK -$ SYSTEM\TLCS_870\Link.lcf
ECHO ****************

ECHO CONVERTING....
TUCONV -$ -Fh20 RAM_Image.abs
ECHO ****************

IF EXIST RAM_Image.h20 COPY RAM_Image.h20 ..\RamImage\C_FEE64_RAM40_ROM8000_TLCS870_86XX29.hex
