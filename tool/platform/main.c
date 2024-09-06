#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <libgen.h>
#include "Logger.h"
#include "zigbee_sbl.h"

int main(int argc, char* argv[])
{
    Logger.setLineWrap(false);
    int version = 0;
    char* firmware = NULL;

    if (argc < 2)
    {
        Logger.writeLog(LOG_INFO, "Error, wrong number of argument. specify filename");
        exit(-1);
    }
    firmware = basename(argv[1]);

    sscanf (firmware,"CC2538_ZNP_%d.bin", &version);

    mt_revision revision;
    execute_read_sys_vsn(NULL, &revision);

    if (version == revision.swRev)
    {
        Logger.writeLog(LOG_INFO, "Skip updating %s", argv[1]);
        return -1;
    }

    Logger.writeLog(LOG_INFO, "Updating %s", argv[1]);
    int retcode = execute_fw_update(argv[1], "/dev/zigbee");

    return 0;
}