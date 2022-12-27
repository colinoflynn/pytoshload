ECHO OFF

FOR %%i IN (asm, lst, rel, abs, err, map, cpl, h20, s32) DO IF EXIST *.%%i DEL *.%%i

