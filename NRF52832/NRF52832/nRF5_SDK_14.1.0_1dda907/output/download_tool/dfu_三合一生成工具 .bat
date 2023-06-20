set APPLICATION_VERSION=0x00000002
set BOOTLOADER_VERSION=0x00000002
set APPLICATION_PATH=application\application.hex
set BOOTLOADER_PATH=bootloader\bootloader.hex
set SOFTDEVICE_PATH=softdevice\s132_nrf52_5.0.0_softdevice.hex

nrfutil pkg generate DFU_file\dfu_combination.zip --bootloader %BOOTLOADER_PATH% --bootloader-version %BOOTLOADER_VERSION% --application %APPLICATION_PATH% --application-version %APPLICATION_VERSION% --softdevice %SOFTDEVICE_PATH% --hw-version 52 --sd-req 0x9D --sd-id 0x9D --key-file vault\priv.pem

PAUSE
EXIT