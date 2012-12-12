:: Build QtDicom module for all configurations and architectures, next
:: replace the <PATH> placeholder in this file to requested location,
:: save the script as Install.bat and use it.

@echo off
@powershell -NoProfile -NonInteractive -ExecutionPolicy Unrestricted -File Install-QtDicom.ps1 -Path <PATH>
@pause
