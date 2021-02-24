#!/bin/sh

#####################################################################
# main processing
#####################################################################

# check user id first
if [ "$(id -u)" != "0" ]
then
    echo "This script can only be run by superuser or sudoer."
    exit 1
fi

DISTRIBUTION=$(uname --)

if [ -f /etc/lsb-release ]; then
    if [ -f /lib/lsb/init-functions ]; then
        DISTRIBUTION=`grep DISTRIB_ID /etc/lsb-release | sed -e 's/DISTRIB_ID=//'`
    fi
elif [ -f /etc/os-release ]; then
    DISTRIBUTION=`grep -w NAME /etc/os-release | sed 's/[^"]*"\([^"]*\)"/\1/'`
    if [ "$DISTRIBUTION" = "Debian GNU/Linux" ]; then
        DISTRIBUTION="Ubuntu"
    fi
fi

if [ "$DISTRIBUTION" != "Ubuntu" -a "$DISTRIBUTION" != "CentOS Linux" ]
then
    echo ""
    echo "Unsupported distribution. Exit now."
    echo ""
    exit 0
fi

value=1000
if [ "$1" != "" ]
then
    value=$1
fi

echo "Setting usbfs memory size to ${value} ..."
sh -c "echo ${value} > /sys/module/usbcore/parameters/usbfs_memory_mb"
echo -n "usbfs memory size has been set to "
cat /sys/module/usbcore/parameters/usbfs_memory_mb
