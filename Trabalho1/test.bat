@echo off
setlocal enabledelayedexpansion

rem Defina o caminho completo para as pastas _in e _out
set "folder_in=%~dp0_in"
set "folder_out=%~dp0_out"
set "program=%~dp0myprogram.exe"

rem Verifica se as pastas _in e _out existem
if not exist "%folder_in%\" (
    echo Pasta _in não encontrada.
    exit /b
)

if not exist "%folder_out%\" (
    echo Pasta _out não encontrada.
    exit /b
)

rem Itera sobre todos os arquivos na pasta _in
for %%F in ("%folder_in%\*") do (
    rem Extrai apenas o nome do arquivo (sem extensão)
    set "filename=%%~nF"
    
    rem Executa o programa com o arquivo de entrada atual
    if exist "%program%" (
        "%program%" < "%%F" > "%folder_out%\!filename!.txt.tmp"

        rem Compara a saída gerada com o arquivo correspondente na pasta _out
        fc /b "%%F" "%folder_out%\!filename!.txt.tmp" > nul
        if errorlevel 1 (
            echo Saída diferente para o arquivo: !filename!
        ) else (
            rem Imprime a saída igual em verde
            echo Saída igual para o arquivo: !filename!
        )

        rem Exclui o arquivo temporário gerado
        del "%folder_out%\!filename!.txt.tmp"
    ) else (
        echo Programa não encontrado: %program%
        exit /b
    )
)

endlocal
