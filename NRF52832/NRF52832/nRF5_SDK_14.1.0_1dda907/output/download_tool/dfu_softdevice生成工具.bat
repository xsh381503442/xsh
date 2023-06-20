set SOFTDEVICE_PATH=softdevice\s132_nrf52_5.0.0_softdevice.hex


nrfutil pkg generate DFU_file\dfu_softdevice.zip --softdevice %SOFTDEVICE_PATH% --hw-version 52 --sd-req 0x9D --sd-id 0x9D --key-file vault\priv.pem



PAUSE
EXIT