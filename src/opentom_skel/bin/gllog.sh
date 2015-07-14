#!/bin/sh 
# This scripts logs the events from GL realted scripts
# the log is enabled only if the GL logging directory is 
# present on SD card.

log_dir="/mnt/sdcard/gl/log"

if test -d ${log_dir} ; then 
    date  >> ${log_dir}/glecho.txt
    cat - >>  ${log_dir}/glecho.txt
fi

date
cat -

