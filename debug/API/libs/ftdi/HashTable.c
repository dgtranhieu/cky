#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "HashTable.h"



static unsigned int _calcHash(char* key)
{
    unsigned int h = 5381; // DJB algorithm
    while(*(key++))
        h = ((h << 5) + h) + (*key);
    return h;
}


static HashHdr_t* _create(unsigned int capacity)
{
	unsigned int i = 0;
	HashHdr_t* header = malloc(sizeof(HashHdr_t));
	if(!header) return NULL;

	if((header->table = malloc(capacity*sizeof(Element_t*))) == NULL)
	{
		free(header->table);
		return NULL;
	}

	header->capacity = capacity;
	header->e_num = 0;

	for(i = 0; i < capacity; i++)
		header->table[i] = NULL;

	return header;
}


static void* _insert(HashHdr_t* header, char* key, void* data)
{
    if (data == NULL) return NULL;
    unsigned int hashKey = _calcHash(key) % header->capacity;
    Element_t* elem = header->table[hashKey];

    while(elem != NULL)
    {
        if(!strcmp(elem->key, key))
        {
            void* ret = elem->data;
            free(elem->data); // @pxhoang: Free prev value
            elem->data = data;
            return ret;
        }
        elem = elem->next;
    }

    // Getting here means the key doesn't already exist
    if((elem = malloc(sizeof(Element_t)+strlen(key)+1)) == NULL)
        return NULL;

    strcpy(elem->key, key);
    elem->data = data;

    // Add the Element_t at the beginning of the linked list
    elem->next = header->table[hashKey];
    header->table[hashKey] = elem;
    header->e_num++;

    return NULL;
}


static void* _get(HashHdr_t* header, char* key)
{
    unsigned int h = _calcHash(key) % header->capacity;
    Element_t* elem = header->table[h];
    while(elem != NULL)
    {
        if(!strcmp(elem->key, key))
            return elem->data;
        elem = elem->next;
    }
    return NULL;
}


static void* _remove(HashHdr_t* header, char* key)
{
    unsigned int hashKey = _calcHash(key) % header->capacity;
    Element_t* elem = header->table[hashKey];
    Element_t* prev = NULL;

    while(elem != NULL)
    {
        if(!strcmp(elem->key, key))
        {
            void* ret = elem->data;
            if(prev != NULL)
                prev->next = elem->next;
            else
                header->table[hashKey] = elem->next;

            free(elem);
            elem = NULL; // @pxhoang
            header->e_num --;
            return ret;
        }
        prev = elem;
        elem = elem->next;
    }

    return NULL;
}


static void _listKeys(HashHdr_t* header, char** keys, size_t len)
{
    if(len < header->e_num)
        return;

    int ki = 0;
    int cap = header->capacity;

    while(--cap >= 0)
    {
        Element_t* elem = header->table[cap];
        while(elem)
        {
            keys[ki++] = elem->key;
            elem = elem->next;
        }
    }
}


static void _listValues(HashHdr_t* header, void** vals, size_t len)
{
    if(len < header->e_num)
        return;

    int vi = 0; // Index to the current string in **v
    int cap = header->capacity;
    while(--cap >= 0)
    {
        Element_t* elem = header->table[cap];
        while(elem)
        {
            vals[vi++] = elem->data;
            elem = elem->next;
        }
    }
}


static Element_t* _iterate(Iterator_t* iterator)
{
    while(iterator->elem == NULL)
    {
        if(iterator->index < iterator->header->capacity - 1)
        {
            iterator->index++;
            iterator->elem = iterator->header->table[iterator->index];
        }
        else
            return NULL;
    }
    Element_t* elem = iterator->elem;
    if(elem)
        iterator->elem = elem->next;

    return elem;
}


static char* _iterateKeys(Iterator_t* iterator)
{
    Element_t* elem = _iterate(iterator);
    return (elem == NULL ? NULL : elem->key);
}


static void* _iterateValues(Iterator_t* iterator)
{
    Element_t* elem = _iterate(iterator);
    return (elem == NULL ? NULL : elem->data);
}


static void clear(HashHdr_t* header, int free_data)
{
    Iterator_t it = HT_ITERATOR(header);
    char* key = _iterateKeys(&it);

    while(key != NULL)
    {
        free_data ? free(_remove(header, key)) : _remove(header, key);
        key = _iterateKeys(&it);
    }
}


static void _destroy(HashHdr_t* header)
{
    clear(header, 1); // Delete and free all.
    free(header->table);
    free(header);
}


_HashTable const HashTable = {_create, _insert, _get, _remove, _destroy, _listKeys, _listValues, _iterateKeys, _iterateValues};
