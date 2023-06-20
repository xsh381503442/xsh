
nrfutil keys generate vault\priv.pem

nrfutil keys display --key sk --format hex vault\priv.pem

nrfutil keys display --key sk --format code vault\priv.pem

nrfutil keys display --key pk --format hex vault\priv.pem

nrfutil keys display --key pk --format code vault\priv.pem

nrfutil keys display --key pk --format code vault\priv.pem --out_file vault\public_key.c

PAUSE
EXIT
