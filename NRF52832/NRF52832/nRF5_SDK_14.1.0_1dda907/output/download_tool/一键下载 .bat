
@echo off
::set Jlink_SN=10000351
::set Jlink_SN=682598860
::set Jlink_SN=10000543

set SETTINGS_VERSION=0x01
set BOOTLOADER_VERSION=0x00000001
set APPLICATION_VERSION=0x0000000E
set PCB_VERSION=0x00000001
set APPLICATION_PATH=application\application.hex
set BOOTLOADER_PATH=bootloader\bootloader.hex
set SOFTDEVICE_PATH=softdevice\s132_nrf52_5.0.0_softdevice.hex

nrfutil settings generate bootloader_settings.hex --family NRF52 --application %APPLICATION_PATH% --application-version %APPLICATION_VERSION% --bootloader-version %BOOTLOADER_VERSION% --bl-settings-version %SETTINGS_VERSION%
mergehex -m bootloader_settings.hex %APPLICATION_PATH% %BOOTLOADER_PATH% -o nrf_bootloader_and_application.hex
mergehex -m nrf_bootloader_and_application.hex %SOFTDEVICE_PATH% -o nrf_bootloader_and_application_softdevice.hex


del PCBA_Factory_Downloader\nrf_bootloader_and_application_softdevice.hex
cp nrf_bootloader_and_application_softdevice.hex PCBA_Factory_Downloader\nrf_bootloader_and_application_softdevice.hex


nrfjprog.exe --recover --log
nrfjprog.exe -e
nrfjprog.exe --memwr 0x10001080 --val %PCB_VERSION% --verify --family NRF52
nrfjprog.exe --memrd 0x10001080 --family NRF52
nrfjprog.exe --program nrf_bootloader_and_application_softdevice.hex --verify
nrfjprog.exe -r

del bootloader_settings.hex
del nrf_bootloader_and_application.hex
del nrf_bootloader_and_application_softdevice.hex



echo ��ϸ�鿴�����Ϣ���Ƿ���ERROR����
echo �����ERROR˵���̼����ز��ɹ�����Ҫ��������
echo �ر������д���
echo �����²�����

PAUSE
EXIT

