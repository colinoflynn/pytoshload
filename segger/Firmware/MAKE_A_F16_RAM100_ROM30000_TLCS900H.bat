ECHO OFF
CLS
CALL PREP_TLCS_900.BAT

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

SET CC_OPT=-$ -DTMP95FW54 -DBUFFER_SIZE=0x100 -w1 -c -l -XS -I FLASH -I SRC -I SYSTEM -I SYSTEM\TLCS_900_H -DINT_ROMBASEADDR=0x030000

ECHO COMPILING...
CC900 %CC_OPT% SYSTEM\TLCS_900_H\System.c
IF ERRORLEVEL 1 PAUSE
CC900 %CC_OPT% FLASH\Flash16.c
IF ERRORLEVEL 1 PAUSE
CC900 %CC_OPT% SRC\Main.c
IF ERRORLEVEL 1 PAUSE
CC900 %CC_OPT% SRC\CRC.c
IF ERRORLEVEL 1 PAUSE
ECHO ****************

ECHO LINKING...
TULINK -$ SYSTEM\TLCS_900_H\Link.lcf
ECHO ****************

ECHO CONVERTING....
TUCONV -$ -Fh20 RAM_Image.abs
ECHO ****************

IF EXIST RAM_Image.h20 COPY RAM_Image.h20 ..\RamImage\A_F16_RAM100_ROM30000_TLCS900H.hex
