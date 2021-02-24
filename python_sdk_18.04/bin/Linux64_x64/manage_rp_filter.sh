#!/bin/sh

#####################################################################
# help functions
#####################################################################

# print help for the script
showHelp()
{
cat << TEXT_HERE

DESCRIPTION
    
    This script allows to manage rp_filter(reverse path filter) mode.
    Allows devices on different subnet to be discovered.

    This script can only be run by the root or sudoer user.

SYNOPSIS

    manage_rp_filter.sh [--iface=<name>][--mode=0|1|2][--permanent][--restart ][--help]

    --iface=<name>          Inteface name on which to set rp_filter mode.
                                If all, then mode will be set for all interface only
                                Default is empty which means entire system interfaces(all,default,lo and so on).

    --mode=0|1|2            Sets the mode of rp_filter:
                                0 - No source validation.
                                1 - Strict mode as defined in RFC3704 Strict Reverse Path.
                                    Each incoming packet is tested against the FIB and if the interface
                                    is not the best reverse path the packet check will fail.
                                    By default failed packets are discarded.
                                2 - Loose mode as defined in RFC3704 Loose Reverse Path.
                                    Each incoming packet's source address is also tested against the FIB
                                    and if the source address is not reachable via any interface
                                    the packet check will fail. (recommended)
                                Default is 1.

    --permanent             Add setting line to /etc/sysctl.conf

    --restart               Restarts network stack after setting rp_filter mode.
                                By default network is not restarted.

    --help                  Shows this help

TEXT_HERE
}

#####################################################################
# main processing
#####################################################################

# check user id first
if [ "$(id -u)" != "0" ]
then
    echo "This script can only be run by superuser or sudoer."
    echo "To see help use --help."
    exit 1
fi

IFACE=""
MODE=1
PERMANENT=0
RESTART=0

# lets parse all the parameters
for i in $*
do
    case $i in
        --iface=*)
            IFACE=${i#*=}
            ;; 
        --mode=*)
            MODE=${i#*=}
            ;;
        --permanent)
            PERMANENT=1
            ;;
        --restart)
            RESTART=1
            ;;
        --help)
            showHelp
            exit 0
            ;;
        *) # unknown option
            showHelp
            exit 1
            ;;
    esac
done

if [ "${MODE}" != "0" -a "${MODE}" != "1" -a "${MODE}" != "2" ]; then
    echo ""
    echo "Supported modes:"
    echo " 0 - No source validation."
    echo " 1 - Strict Reverse Path."
    echo " 2 - Loose Reverse Path."
    echo ""
    usersAnswer=""
    until [ "${usersAnswer}" = "0" -o "${usersAnswer}" = "1" -o "${usersAnswer}" = "2" ]
    do
        printf "Enter operation [0|1|2]. Default is 0: "
        read usersAnswer
        if [ -z "${usersAnswer}" ]
        then
            usersAnswer="0"
        fi
    done
    MODE=${usersAnswer}
fi

MODE_TEXT="No source validation"
if [ "${MODE}" = "1" ]; then
    MODE_TEXT="Strict Reverse Path"
elif [ "${MODE}" = "2" ]; then
    MODE_TEXT="Loose Reverse Path"
fi

# print what is going to be done
echo "Setting rp_filter mode to '${MODE_TEXT}'...."
if [ "${IFACE}" = "" ]; then
    echo "For all the system's interfaces"
else
    echo "For interface: '${IFACE}'"
fi
if [ ${PERMANENT} -ne 0 ]; then
    echo "Setting will be written to /etc/sysctl.conf"
fi
if [ ${RESTART} -ne 0 ]; then
    echo "The network stack will be restarted"
fi

DIR=/proc/sys/net/ipv4/conf
INTERFACES=$(ls ${DIR})

# check if user wants particular interface
if [ "${IFACE}" = "" ]; then
    for i in ${INTERFACES}; do
        if [ -f ${DIR}/${i}/rp_filter ]; then
            echo ${MODE} > ${DIR}/${i}/rp_filter;
        fi

        # check if user wants to add setting into /etc/sysctl.conf
        if [ ${PERMANENT} -ne 0 ]; then
            if grep --silent ^net.ipv4.conf.${i}.rp_filter /etc/sysctl.conf; then
                sed -i "s/^net.ipv4.conf.${i}.rp_filter.*/net.ipv4.conf.${i}.rp_filter = ${MODE}/g" /etc/sysctl.conf
            else
                echo "" >> /etc/sysctl.conf
                echo "net.ipv4.conf.${i}.rp_filter = ${MODE}" >> /etc/sysctl.conf
            fi
        fi
    done
else
    if [ -f "/proc/sys/net/ipv4/conf/${IFACE}/rp_filter" ]; then
        echo ${MODE} > "/proc/sys/net/ipv4/conf/${IFACE}/rp_filter";
        
        # check if user wants to add setting into /etc/sysctl.conf
        if [ ${PERMANENT} -ne 0 ]; then
            if grep --silent ^net.ipv4.conf.${IFACE}.rp_filter /etc/sysctl.conf; then
                sed -i "s/^net.ipv4.conf.${IFACE}.rp_filter.*/net.ipv4.conf.${IFACE}.rp_filter = ${MODE}/g" /etc/sysctl.conf
            else
                echo "" >> /etc/sysctl.conf
                echo "net.ipv4.conf.${IFACE}.rp_filter = ${MODE}" >> /etc/sysctl.conf
            fi
        fi
    else
        echo "interface ${IFACE} does not exist."
    fi
fi

if [ ${RESTART} -ne 0 ]; then
    if [ -f /etc/init.d/network ]; then
        service network restart 
    else
        service network-manager restart
    fi
fi

echo "Setting rp_filter mode to '${MODE_TEXT}' DONE"
