# Scripts configuration values

# A remote machine with parallel port is used for hardware programming
# Secret data is stored in 'secret' file
source "$(dirname "$0")/secret"
REMOTE_PROGRAMING_HOST=$SECRET_HOST
REMOTE_PROGRAMING_USER=$SECRET_USER
REMOTE_PROGRAMING_PASSWD=$SECRET_PASSWD

# Compiled firmware
BINDIR="$(dirname "$0")/../bin"
FIRMWARE='example.hex'

# Debugging serial device
DEBUG_DEV='/dev/ttyUSB0'

# Micro controller model
PIC='PIC18F4550'
