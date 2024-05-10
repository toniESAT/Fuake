@REM Compilación y Enlace con biblioteca gráfica.
@cls
@echo ---------------------------------------------------
@echo  ESAT Curso 2023-2024 Asignatura PRG Primero
@echo ---------------------------------------------------
@echo  Proceso por lotes iniciado.
@echo ---------------------------------------------------
@echo off

cl /std:c++17 /nologo /Zi /GR- /EHs /MD %1 /I "include/" /I "AMath_Lib/" /I "AMath_Lib/include" /I "../ESAT_rev248/include" "../ESAT_rev248\bin\ESAT.lib" opengl32.lib user32.lib gdi32.lib shell32.lib Ws2_32.lib 
 
@echo ---------------------------------------------------
@echo  Proceso por lotes finalizado.
@echo ---------------------------------------------------