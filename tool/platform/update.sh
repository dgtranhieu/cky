REVISION=20200327
SMART_LIVING_DIR=/home/panl/SmartLiving_RAS
FW_DIR=$SMART_LIVING_DIR/zigbee_gw/firmware
FIRMWARE=${FW_DIR}/CC2538_ZNP_${REVISION}.bin

start() {
    $FW_DIR/upgrade_tool.bin $FIRMWARE
}

case "$1" in
    'start')
        start
        ;;
    *)
    echo
    echo "Usage: $0 { start | stop | restart | status }"
    echo
    exit 1
    ;;
esac
exit 0