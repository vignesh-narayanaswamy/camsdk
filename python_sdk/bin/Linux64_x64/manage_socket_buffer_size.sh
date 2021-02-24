#!/bin/sh

##############################################################
# main processing
##############################################################

# check user id first
if [ "$(id -u)" != "0" ]
then
    echo "This script can only be run by superuser or sudoer."
    exit 1
fi

if [ "$(uname --)" = "Linux" ]
then
    DIR_PATH=/proc/sys/net/core

    SIZE=10485760 # 10MB
    # check if user wants their own value
    if [ "$1" != "" ]
    then
        SIZE=$1
    fi

    # check if we can change size
    if [ ! -w ${DIR_PATH}/wmem_max ]
    then
        echo "Cannot write to ${DIR_PATH}/wmem_max!"
        exit 1
    fi

    if [ ! -w ${DIR_PATH}/rmem_max ]
    then
        echo "Cannot write to ${DIR_PATH}/rmem_max"
        exit 1
    fi

    echo "Setting socket write maximum buffer size to ${SIZE} bytes"
    echo ${SIZE} > ${DIR_PATH}/wmem_max
    echo "Setting socket read maximum buffer size to ${SIZE} bytes"
    echo ${SIZE} > ${DIR_PATH}/rmem_max
    exit 0
fi

echo "Error! Unknown System!"
exit 1

