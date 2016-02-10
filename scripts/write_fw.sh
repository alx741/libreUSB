#! /bin/sh

# Send compiled .hex file to the programming machine and write the firmware to
# the micro controller

source "$(dirname "$0")/config.sh"


cd $BINDIR

# Check firmware is compiled
if [ ! -f "$FIRMWARE" ]; then
    echo "[!] There is no firmware file"
    exit 1
fi

# Send firmware to programming machine
ftp -n -v $REMOTE_PROGRAMING_HOST << EOT
ascii
user $REMOTE_PROGRAMING_USER $REMOTE_PROGRAMING_PASSWD
prompt
delete $FIRMWARE
put $FIRMWARE
bye
EOT



# Perform hardware flash

# Check micro controller is writable
check_hardware_result=$(
sshpass -p "$REMOTE_PROGRAMING_PASSWD" ssh \
"$REMOTE_PROGRAMING_USER"@"$REMOTE_PROGRAMING_HOST" << ENDSSH
odyssey "$PIC" check
ENDSSH
)

if [[ "$check_hardware_result" != "Chip Rev: 0x7" ]]
then
    echo
    echo
    echo "[!] $PIC Chip is disconnected"
    echo
    exit 1
fi


# Write firmware to micro controller
sshpass -p "$REMOTE_PROGRAMING_PASSWD" ssh \
"$REMOTE_PROGRAMING_USER"@"$REMOTE_PROGRAMING_HOST" << ENDSSH
odyssey "$PIC" erase
odyssey "$PIC" write "$FIRMWARE"
rm "$FIRMWARE"
ENDSSH
