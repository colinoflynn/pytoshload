ECHO OFF
CLS
CALL PREP_TX19.BAT

ECHO CLEARING...
CALL CLEAN.BAT
ECHO ****************

GOTO CONT

-$    silent
-w0   warning level
-c    create object
-XS   optimize size
-l    create list file
-o    output file name
-O0   no optimization
-XA4  alignment area section 4 bytes
-Rle  little endian
-g    debug info
-Nr32 default function qualifier
-J    select kanji mode

:cont

SET CC_OPT=-$ -DBUFFER_SIZE=0x200 -DBUFFER_ADR=0xFFFFA000 -XA4 -XC4 -XD4 -ZC far -ZD far -ZA far -Rle -c -l -Nr32 -XS -I FLASH -I SRC -I SYSTEM -I SYSTEM\TX19

ECHO COMPILING...
CC9i %CC_OPT% SYSTEM\TX19\System.c
IF ERRORLEVEL 1 PAUSE
CC9i %CC_OPT% FLASH\Flash32.c
IF ERRORLEVEL 1 PAUSE
CC9i %CC_OPT% SRC\Main.c
IF ERRORLEVEL 1 PAUSE
CC9i %CC_OPT% SRC\CRC.c
IF ERRORLEVEL 1 PAUSE
ECHO ****************

ECHO LINKING...
TULINK9i -$ SYSTEM\TX19\Link.lcf
ECHO ****************

ECHO CONVERTING....
TUCONV -$ -Fs32 RAM_Image.abs
ECHO ****************

IF EXIST RAM_Image.s32 COPY RAM_Image.s32 ..\RamImage\B_F32_RAMFFFF8000_ROM0_TX19.mot
