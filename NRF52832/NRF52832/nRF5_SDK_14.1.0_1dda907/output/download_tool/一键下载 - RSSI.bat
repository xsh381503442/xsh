
@echo off
::set Jlink_SN=10000351
::set Jlink_SN=682598860
::set Jlink_SN=10000543

set SETTINGS_VERSION=0x01
set BOOTLOADER_VERSION=0x0100006D
set APPLICATION_VERSION=0x0100006D
set PCB_VERSION=0x01000002
set APPLICATION_PATH=RSSI\RSSI.hex
set BOOTLOADER_PATH=bootloader\bootloader.hex
set SOFTDEVICE_PATH=softdevice\s132_nrf52_5.0.0_softdevice.hex


nrfjprog.exe --recover --log
nrfjprog.exe -e
nrfjprog.exe --memwr 0x10001080 --val %PCB_VERSION% --verify --family NRF52
nrfjprog.exe --memrd 0x10001080 --family NRF52
nrfjprog.exe --program %APPLICATION_PATH% --verify
nrfjprog.exe -r



echo 仔细查看输出信息中是否有ERROR错误
echo 如果有ERROR说明固件下载不成功，需要重新下载
echo 关闭命令行窗口
echo 继续下步操作

PAUSE
EXIT



