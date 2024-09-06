#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "HashTable.h"
#include "Logger.h"
#include "SafeHash.h"


static SafeHashHdr_t* _instance(unsigned int capacity)
{
    SafeHashHdr_t* header = calloc(1, sizeof(SafeHashHdr_t));
    header->maxEntry = capacity;
    header->hashHdr = HashTable.create(header->maxEntry);

    return header;
}


static void _destroy(SafeHashHdr_t* header)
{
    HashTable.destroy(header->hashHdr);
    free(header);
    header = NULL;
}


static void* _create(int len)
{
    void* elem = calloc(1, len);
    return elem;
}


static void _insert(SafeHashHdr_t* header, Key_t key, void* entry)
{
    void* elem = HashTable.get(header->hashHdr, key.data);
    if (elem != NULL)
        return;

    HashTable.insert(header->hashHdr, key.data, entry);
}


static void _remove(SafeHashHdr_t* header, Key_t key)
{
    void* elem = HashTable.get(header->hashHdr, key.data);
    if (elem == NULL)
        return;

    HashTable.remove(header->hashHdr, key.data);
}


static void* _find(SafeHashHdr_t* header, Key_t key)
{
    void* entry = HashTable.get(header->hashHdr, key.data);
    if (entry == NULL)
        return NULL;

    return entry;
}


static void _update(SafeHashHdr_t* header, Key_t key, void* entry)
{
    HashTable.remove(header->hashHdr, key.data);
    HashTable.insert(header->hashHdr, key.data, entry);
}


static void _display(SafeHashHdr_t* header)
{
    Iterator_t it = HT_ITERATOR(header->hashHdr);
    char* key = HashTable.iterateKeys(&it);
    while(key != NULL)
    {
        Logger.writeLog(LOG_DEBUG, "SafeHash%s: key = %s", __FUNCTION__, key);
        key = HashTable.iterateKeys(&it);
    }
}


static Key_t _iterateKeys(Iterator_t* iterator)
{
    Key_t key;
    char* cKey = HashTable.iterateKeys(iterator);
    if (cKey == NULL)
    {
        key.len = 0;
        memset(key.data, 0, 256);
    }
    else
    {
        memcpy(key.data, cKey, 256);
        key.len = strlen(key.data);
    }

    return key;
}


SafeHash_t const SafeHash = {_instance, _destroy, _create, _insert, _remove, _find, _update, _iterateKeys, _display};
