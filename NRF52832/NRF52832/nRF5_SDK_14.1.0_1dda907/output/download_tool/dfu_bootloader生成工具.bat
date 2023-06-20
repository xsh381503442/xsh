set BOOTLOADER_VERSION=0x00000004
set BOOTLOADER_PATH=bootloader\bootloader.hex


nrfutil pkg generate DFU_file\dfu_bootloader.zip --bootloader %BOOTLOADER_PATH% --bootloader-version %BOOTLOADER_VERSION%  --hw-version 52 --sd-req 0x9D --key-file vault\priv.pem
PAUSE
EXIT