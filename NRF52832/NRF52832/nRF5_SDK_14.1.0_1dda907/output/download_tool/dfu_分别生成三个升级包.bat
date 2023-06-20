set APPLICATION_VERSION=0x99999999
set APPLICATION_PATH=application\application.hex
set BOOTLOADER_VERSION=0x99999999
set BOOTLOADER_PATH=bootloader\bootloader.hex
set SOFTDEVICE_PATH=softdevice\s132_nrf52_5.0.0_softdevice.hex

nrfutil pkg generate DFU_file\dfu_application.zip --application %APPLICATION_PATH% --application-version %APPLICATION_VERSION%  --hw-version 52 --sd-req 0x9D --key-file vault\priv.pem
nrfutil pkg generate DFU_file\dfu_bootloader.zip --bootloader %BOOTLOADER_PATH% --bootloader-version %BOOTLOADER_VERSION%  --hw-version 52 --sd-req 0x9D --key-file vault\priv.pem
nrfutil pkg generate DFU_file\dfu_softdevice.zip --softdevice %SOFTDEVICE_PATH% --hw-version 52 --sd-req 0x9D --sd-id 0x9D --key-file vault\priv.pem


PAUSE
EXIT