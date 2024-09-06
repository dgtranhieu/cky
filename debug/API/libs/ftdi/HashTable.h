#ifndef HASH_TABLE_H
#define HASH_TABLE_H


typedef struct Element_t
{
	struct Element_t* next;
	void* data;
	char key[];
} Element_t;


typedef struct HashHdr_t
{
	unsigned int capacity;
	unsigned int e_num;
	Element_t** table;
}HashHdr_t;


typedef struct Iterator_t
{
    HashHdr_t* header;
    unsigned int index;
    Element_t* elem;
}Iterator_t;
#define HT_ITERATOR(header) {header, 0, header->table[0]}


typedef struct _HashTable
{
    HashHdr_t* (*const create)(unsigned int capacity);
    void* (*const insert)(HashHdr_t* header, char* key, void* data);
    void* (*const get)(HashHdr_t* header, char* key);
    void* (*const remove)(HashHdr_t* header, char* key);
    void (*const destroy)(HashHdr_t* header);
    void (*const listKeys)(HashHdr_t* header, char** keys, size_t len);
    void (*const listValues)(HashHdr_t* header, void** vals, size_t len);
    char* (*const iterateKeys)(Iterator_t* iterator);
    void* (*const iterateValues)(Iterator_t* iterator);
} _HashTable;


extern _HashTable const HashTable;

#endif // HASH_TABLE_H