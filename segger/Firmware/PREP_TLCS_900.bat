@ECHO OFF
SET THOME=C:\TOOL\C\TOSHIBA\TLCS900_V120\T900
IF "_PREP_TLCS900_" == "%_PREP_TLCS900_%" GOTO CONT
SET PATH=%PATH%;%THOME%\BIN
:CONT
SET _PREP_TLCS900_=_PREP_TLCS900_