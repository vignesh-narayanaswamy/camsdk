#!/bin/sh

#####################################################################
# help functions
#####################################################################

# print help for the script
showHelp()
{
cat << TEXT_HERE

DESCRIPTION
    
    This script allows to manage udev rules for USB devices,
    particulaly, USB3 Vision devices.
    You have to add an udev rule to the system to allow regular user
    to access a device. This script uses vendor ID as a match key in the rule.

    Vendor ID can be found by calling lsusb command, or every USB3 Vision device
    should have label this USB3 Vision GUID (first four characters is vendor ID)

    This script can only be run by the root or sudoer user.

SYNOPSIS

    manage_udevrules.sh [--add=<vendorID>][--remove=<vendorID>][--removeall][--help]

    --add=<vendorID>       Create a rule file for particular vendor id
    --remove=<vendorID>    Remove a rule file for particular vendor id
    --removeall            Remove all the rule files for USB3 Vision devices
    --help                 Show this help

TEXT_HERE
}

# define variables here
ruleDirectory="/etc/udev/rules.d"
imperxId=2db2
vendorId=""
toDo=""

createRuleFile()
{
    # retrieve udev version add generate particular rule
    udevVersion=$(udevadm --version)
    if [ $udevVersion -gt 147 ]
    then

> $1
cat > $1 <<TEXT_HERE
ACTION=="add", SUBSYSTEM=="usb", ENV{DEVTYPE}=="usb_device", ATTRS{idVendor}=="$2", MODE="0666", GROUP="plugdev"
TEXT_HERE

    else

> $1
cat > $1 <<TEXT_HERE
SUBSYSTEM=="usb", SYSFS{idVendor}=="$2", MODE="0666"
TEXT_HERE

    fi

    # set the permission to the file
    chmod 0644 "$1"
}

# check if vendor id is correct
isValisVID() # USAGE: isValisVID VID(in)
{
    case $1 in
        [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]) return 0;;
        *) return 1;;
    esac
}

# create file name for rule
ruleFileName() # USAGE: ruleFileName VID(in) [NAME(out)]
{
    local vidName=""
    vidName=$(echo $1 | tr 'A-F' 'a-f')

    # check if second parameter is set and is not empty
    if [ "${2+x}" = "x" -a -n "${2}" ]
    then
        local __result__=$2
        eval $__result__="$ruleDirectory/81-u3vdevice-$vidName.rules"
    else
        echo "$ruleDirectory/81-u3vdevice-$vidName.rules"
    fi
}

# trigger udev
triggerUdev() # USAGE: triggerUdev action(in) [VID(in)] 
{
    if which udevadm >/dev/null 2>&1
    then
        if [ "${2+x}" = "x" -a -n "${2}" ]
        then
            udevadm trigger -c "$1" -s usb -a idVendor="$2"
        else
            udevadm trigger -c "$1" -s usb
        fi
    fi
}

# add a rule
addVendorId() # USAGE: addVendorId VID(in)
{
    local fileName
    if [ ! -d "$ruleDirectory" ]
    then
        echo "Unable to find the folder that contains the rules ($ruleDirectory)."
        exit 3
    fi

    if ! which udevadm >/dev/null 2>&1
    then
        echo "Unable to find the udevadm tool to retrieve the current version of udev"
        echo "that is installed on this distribution. You must add the rule manually." 
        exit 3
    fi
 
    # return name of the file to be added
    ruleFileName "$1" fileName

    echo "Adding rules for vendor ID $1."

    # rmove any existing rules with this name first
    rm -f $fileName

    # create a new rule file for vendor Id in "$1"
    createRuleFile "$fileName" "$1"

    # reload the rules for the usb subsystem
    triggerUdev add "$1"
    echo "The $fileName rule has been created."
}

# remove a rule
removeVendorId() # USAGE: removeVendorId VID(in)
{
    local fileName
    if [ -d "$ruleDirectory" ]
    then
        # return name of the file to be removed
        ruleFileName "$1" fileName
        echo "Removing the rule for vendor ID $1."
        rm -f $fileName

        # reload the rules for the usb subsystem
        triggerUdev change "$1"

        echo "The rule for vendor ID $1 has been removed."
    fi
}

# remove all the rules except imperx one
removeAllVendorIds() # USAGE: removeAllVendorIds
{
    local fileName
    if [ -d "$ruleDirectory" ]
    then
        # remove all files that match our pattern
        echo "Removing all rules."
        ruleFileName "????" fileName
        rm -f $fileName

        # add a rule for imperx back
        ruleFileName "$imperxId" fileName
        createRuleFile "$fileName" "$imperxId"

        # reload the rules for the usb subsystem
        triggerUdev change

        echo "All rules have been removed."
    fi
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

# lets parse all the parameters
for i in $*
do
    case $i in
        --addImperx)
            vendorId=$imperxId
            toDo="add"
            ;;
        --add=0x*)
            vendorId=${i#*x}
            toDo="add"
            ;;
        --add=0X*)
            vendorId=${i#*X}
            toDo="add"
            ;;
        --add=*)
            vendorId=${i#*=}
            toDo="add"
            ;;
        --add)
            vendorId=""
            toDo="add"
            ;;
        --removeImperx)
            vendorId=$imperxId
            toDo="remove"
            ;;
        --remove=0x*)
            vendorId=${i#*x}
            toDo="remove"
            ;;
        --remove=0X*)
            vendorId=${i#*X}
            toDo="remove"
            ;;
        --remove=*)
            vendorId=${i#*=}
            toDo="remove"
            ;;
        --remove)
            vendorId=""
            toDo="remove"
            ;;
        --removeall)
            vendorId=""
            toDo="removeAll"
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

# check what user wants
if [ -z "$toDo" ] # if it is empty ask user what to do
then
    echo ""
    echo "The following operations can be performed by this script:"
    echo ""
    echo "0 - Add a rule for a device."
    echo "1 - Remove a rule for a device."
    echo "2 - Remove all rules from the system."
    echo ""
    usersAnswer=""
    until [ "$usersAnswer" = "0" -o "$usersAnswer" = "1" -o "$usersAnswer" = "2" ]
    do
        printf "Enter operation [0|1|2]. Default is 0: "
        read usersAnswer
        if [ -z "$usersAnswer" ]
        then
            usersAnswer="0"
        fi
    done

    # analyze the answer
    case $usersAnswer in
        0) toDo="add" ;;
        1) toDo="remove" ;;
        2) toDo="removeAll" ;;
    esac
fi

# ask for the vendorId if it is empty
if [ "$toDo" != "removeAll" ] && ! isValisVID "$vendorId"
then
    usersAnswer=""
    while [ "$usersAnswer" = "" ]
    do
        printf "Enter a vendor ID containing four hexadecimal digits: "
        read usersAnswer
        if [ -z "$usersAnswer" ]
        then
            usersAnswer=""
        else
            if ! isValisVID "$usersAnswer"
            then
                usersAnswer=""
            fi
        fi
    done
    vendorId=$usersAnswer
fi

# do the operation (toDo)
echo ""
case $toDo in
    add) addVendorId $vendorId ;;
    remove) removeVendorId $vendorId ;;
    removeAll) removeAllVendorIds ;;
esac
echo ""
