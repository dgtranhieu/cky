#include <stdio.h>
#include "Command.h"


/**-------------------------------------------------------------------------------------------------
 * [description]
 *------------------------------------------------------------------------------------------------*/

static Cmd_t* _create(CmdSeverity_t severity, void (*execute)(void* header, void* msg))
{
    Cmd_t* cmdHdr = malloc(sizeof(Cmd_t));
    cmdHdr->severity = severity;
    cmdHdr->execute = execute;

    return cmdHdr;
}

static void _destroy(Cmd_t* header)
{
    if (header->msg)
        free(header->msg);

    free(header);

    header->msg = NULL;
    header = NULL;
}

_Cmder const Cmder = {_create, _destroy};