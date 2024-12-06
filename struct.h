#ifndef _STRUCT_H
#define _STRUCT_H

struct ListEntry {
    ListEntry() = default;
    ListEntry* prev = this;
    ListEntry* next = this;
};

typedef ListEntry* PListEntry;

inline bool
RemoveEntryList(PListEntry entry) {
    PListEntry prev;
    PListEntry next;

    next = entry->next;
    prev = entry->prev;
    if ((next->prev != entry) || (prev->next != entry)) {
        ;
    }
    prev->next = next;
    next->prev = prev;
    return next == prev;
}

inline void InsertTailList(PListEntry head, PListEntry entry) {
    PListEntry prev = head->prev;
    if (prev->next != head) {
        ;
    }
    entry->next = head;
    entry->prev = prev;
    prev->next = entry;
    head->prev = entry;
    return;
}

#endif
/// @}