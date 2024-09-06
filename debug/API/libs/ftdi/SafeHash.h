#ifndef SAFE_HASH_H
#define SAFE_HASH_H
#include "UniqueKey.h"
#include "HashTable.h"

typedef struct SafeHashHdr_t
{
    HashHdr_t* hashHdr;
    int maxEntry;
}SafeHashHdr_t;

#define SH_ITERATOR(header) {header->hashHdr, 0, header->hashHdr->table[0]}



typedef struct SafeHash_t
{
    SafeHashHdr_t* (*const instance)(unsigned int capacity);
    void (*const destroy)(SafeHashHdr_t* header);

    void* (*const create)(int len);

    void (*const insert)(SafeHashHdr_t* header, Key_t key, void* entry);
    void (*const remove)(SafeHashHdr_t* header, Key_t key);

    void* (*const find)(SafeHashHdr_t* header, Key_t key);
    void (*const update)(SafeHashHdr_t* header, Key_t key, void* entry);

    Key_t (*const iterateKeys)(Iterator_t* iterator);

    void (*const display)(SafeHashHdr_t* header);

}SafeHash_t;




extern SafeHash_t const SafeHash;


#endif // SAFE_HASH_H