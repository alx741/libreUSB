#! /bin/sh
source "$(dirname "$0")/config.sh"

sudo picocom -c --imap lfcrlf "$DEBUG_DEV"
