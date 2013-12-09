:: Builds QtDicom module for all configurations, architectures, and Qt
:: libraries available

@echo off
@powershell -NoProfile -NonInteractive -ExecutionPolicy Unrestricted -Command "& '..\Scripts\RebuildAndInstall-All.ps1' -QtDirPrefix 'E:\Qt' -OutputDirPrefix 'E:\QtDicom' -Versions '4.8.1','4.8.5'"
@pause
