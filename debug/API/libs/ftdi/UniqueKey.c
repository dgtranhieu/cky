#include "UniqueKey.h"






/**-------------------------------------------------------------------------------------------------
 * CmdKey_t
 *------------------------------------------------------------------------------------------------*/

Key_t _cmdKeyRender(Kid_t id)
{
    Key_t key;
    int len = sprintf(key.data, "id-cmd-%d-%d-%d", (int)id.cmd.domain, (int)id.cmd.mode, (int)id.cmd.cmdId);
    key.len = len;

    return key;
}


Kid_t _cmdKeyParser(Key_t key)
{
    Kid_t kid;

    char* tokens[8] = {0};
    int   index     = 0;

    tokens[index] = strtok(key.data, "-");
    while (tokens[index] != NULL)
    {
        index++;
        tokens[index] = strtok(NULL, "-");
    }

    kid.cmd.domain = (int)atoi(tokens[2]);
    kid.cmd.mode   = (int)atoi(tokens[3]);
    kid.cmd.cmdId  = (int)atoi(tokens[4]);

    return kid;
}


Key_t _cmdKeyAdapter(char* data)
{
    Key_t key;
    key.len = (int)strlen(data) + 1; // @pxhoang: Pitfall, null terminator must be counted
    memcpy(key.data, data, key.len);
    return key;
}


/**-------------------------------------------------------------------------------------------------
 * Itemkey
 * id-item-<mac>-<eid>
 *------------------------------------------------------------------------------------------------*/

Key_t _itemKeyRender(Kid_t id)
{
    Key_t key;
    int len = sprintf(key.data, "id-item-%llX-%d", id.item.mac, id.item.eid);
    key.len = len;
    return key;
}


Kid_t _itemKeyParser(Key_t key)
{
    Kid_t kid;
    sscanf (key.data,"id-item-%llX-%d", &kid.item.mac, &kid.item.eid);
    return kid;
}


Key_t _itemKeyAdapter(char* data)
{
    Key_t key;
    key.len = (int)strlen(data) + 1; // @pxhoang: Pitfall, null terminator must be counted
    memcpy(key.data, data, key.len);
    return key;
}


/**-------------------------------------------------------------------------------------------------
 * clusterKey
 * id-cluster-i-mac-eid-cid
 * id-cluster-o-mac-eid-cid
 *------------------------------------------------------------------------------------------------*/

// Key_t _clusterKeyRender(Kid_t id)
// {
//     Key_t key;
//     int len = sprintf(key.data, "id-cluster-%d-%llX-%d-%d", id.cluster.type, id.cluster.mac, id.cluster.eid, id.cluster.cid);
//     key.len = len;
//     return key;
// }


// Kid_t _clusterKeyParser(Key_t keyEntry)
// {
//     Kid_t kid;
//     return kid;
// }


/**-------------------------------------------------------------------------------------------------
 * AttrKey_t
 * id-cluster-i-mac-eid-cid-aid
 * id-cluster-o-mac-eid-cid-aid
 *------------------------------------------------------------------------------------------------*/

// Key_t _attrKeyRender(Kid_t id)
// {
//     Key_t key;
//     int len = sprintf(key.data, "id-cluster-%d-%llX-%d-%d-%d", id.attr.type, id.attr.mac, id.attr.eid, id.attr.cid, id.attr.aid);
//     key.len = len;
//     return key;
// }


// Kid_t _attrKeyParser(Key_t keyEntry)
// {
//     Kid_t kid;
//     return kid;
// }


/**-------------------------------------------------------------------------------------------------
 * uuid
 * uuid-mac-eid-plg
 *------------------------------------------------------------------------------------------------*/

Key_t _uuidKeyRender(Kid_t kid)
{
    Key_t key;
    int len = sprintf(key.data, "uuid-%llX-%u-%d", kid.uuid.mac, kid.uuid.eid, kid.uuid.plg);
    key.len = len;
    return key;
}


Kid_t _uuidKeyParser(Key_t key)
{
    Kid_t kid;
    sscanf (key.data,"uuid-%llX-%u-%d", &kid.uuid.mac, &kid.uuid.eid, &kid.uuid.plg);
    return kid;
}


Key_t _uuidKeyAdapter(char* data)
{
    Key_t key;
    key.len = (int)strlen(data) + 1; // @pxhoang: Pitfall, null terminator must be counted
    memcpy(key.data, data, key.len);
    return key;
}


/**-------------------------------------------------------------------------------------------------
 * Export
 *------------------------------------------------------------------------------------------------*/

KeyOps_t keyOps[] = {
    {"cmd"  ,_cmdKeyRender   ,_cmdKeyParser   ,_cmdKeyAdapter},
    {"item" ,_itemKeyRender  ,_itemKeyParser  ,_itemKeyAdapter},
    {"uuid" ,_uuidKeyRender  ,_uuidKeyParser  ,_uuidKeyAdapter}
};


KeyOps_t _typeof(const char* name)
{
    int len = sizeof(keyOps) / sizeof(KeyOps_t);

    for (int i = 0; i < len; i++)
    {
        if (strcmp(name, keyOps[i].name) == 0)
        {
            return keyOps[i];
        }
    }

    return (KeyOps_t){"Invalid", NULL, NULL};
}

UniqueKey_t const UniqueKey = {_typeof};