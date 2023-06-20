set APPLICATION_VERSION=0x00000004
set APPLICATION_PATH=application\application.hex


nrfutil pkg generate DFU_file\dfu_application.zip --application %APPLICATION_PATH% --application-version %APPLICATION_VERSION%  --hw-version 52 --sd-req 0x9D --key-file vault\priv.pem


PAUSE
EXIT