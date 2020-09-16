#include "SlabMemory.hpp"
#include "string.h"

SlabMemory::SlabMemory(Slab *slabStart, uint16_t slabCount) :
    mSlabs(slabStart),
    mSlabCount(slabCount) {}

void SlabMemory::reset() {
    for(uint8_t i = 0; i < mSlabCount; i++) {
        mSlabs[i].page = 0;
    }
}

// Initialize a new slab for the provided address. 
// Sets the slab page number, initializes it to 0, and then
// tries to read in a PROGMEM value. If that fails, the value is
// set to 0.
void SlabMemory::initSlab(Slab &slab, uint16_t addr) {
    slab.page = addr >> 4;
    uint16_t pageStart = addr & 0xFF0;
    for(uint16_t i = 0; i < 16; i++) {
        if(!externalRead(pageStart + i, &slab.data[i], 1)) {
            slab.data[i] = 0;
        }
    }
}

// Try to find an already-allocated slab for the provided address.
// This works by searching through the current list of slabs for as
// long as we see slabs with a page identifier != 0.
// If a slab is found, a pointer to it is returned.
// If we reach an unused slab, we return that, the caller can decide
// to allocate it if desired.
// If the end of the list is reached with no match, NULL is returned.
Slab* SlabMemory::findWriteSlab(uint16_t addr) {
    uint8_t page = addr >> 4;
    for(uint8_t i = 0; i < mSlabCount; i++) {
        if(mSlabs[i].page == 0) initSlab(mSlabs[i], addr);
        if(mSlabs[i].page == page) return &mSlabs[i];
    }
    return NULL;
}

// Returns the first slab that would be needed for a read of the requested
// size, starting from addr.
// If no slab covers the requested range, NULL is returned.
Slab* SlabMemory::firstReadSlab(uint16_t addr, uint8_t size) {
    uint8_t minPage = addr >> 4;
    uint8_t maxPage = (addr+size) >> 4;
    Slab* found = NULL;
    for(uint8_t i = 0; i < mSlabCount && mSlabs[i].page != 0; i++) {
        Slab &slab = mSlabs[i];
        if (slab.page >= minPage && slab.page <= maxPage) {
            if(found == NULL || slab.page < found->page) {
              found = &mSlabs[i];
            }
        }
    }
    return found;
}


// Read some bytes from memory.
// addr, dest, and size parameters are updated as function progresses
// to track progress.
// This function should handle ranges that are covered by a mixture of slabs
// and pgm.
bool SlabMemory::read(uint16_t addr, uint8_t* dest, uint8_t size) {
    if(size == 0) return true;

    Slab* slab = NULL;

    // In this loop, we repeat the following:
    // 1. Find the earliest-in-memory slab covering the current range.
    // 2. If the slab isn't at the beginning, copy any program data that's before it.
    // 3. Copy the slab data up to the end of this slab.
    do {
        slab = firstReadSlab(addr, size);
        if(slab) {
            // Copy any pgm data up to slab start.
            if(addr < slab->page * 16) {
                uint8_t pgmToRead = (slab->page * 16) - addr;
                if(!externalRead(addr, dest, pgmToRead)) return false;
                addr += pgmToRead;
                dest += pgmToRead;
                size -= pgmToRead;
            }
            
            // Copy slab data
            uint8_t slabToRead = 16 - (addr & 0xF);
            if(slabToRead > size) slabToRead = size;
            memcpy(dest, &(slab->data[addr & 0xF]), slabToRead);
            addr += slabToRead;
            dest += slabToRead;
            size -= slabToRead;
        }
    } while(size > 0 && slab);

    // After the final slab is handled, this handles any remaining pgm data that
    // may need to be written.
    return size > 0 ? externalRead(addr, dest, size) : true;
}

// writeMem finds or allocates a slab of memory and
// updates the provided value in it.
// If the slab allocator is full and we need a new one,
// the program halts and the screen displays a message,
// the provided program will need more slabs to run successfully.
// If an unallocated page is returned, its page is set to the
// page for the requested address. 
// If the page overlaps with any program memory, the program
// memory is copied into the slab.
bool SlabMemory::write(uint16_t addr, uint8_t* src, uint8_t size) {
    do {
        // Find or initialize the next slab to write to.
        Slab *slab = findWriteSlab(addr);
        if(!slab) return false;

        // Write as much into the slab.
        uint8_t slabToWrite = 16 - (addr & 0xF);
        if(slabToWrite > size) slabToWrite = size;
        memcpy(&(slab->data[addr & 0xF]), src, slabToWrite);
        addr += slabToWrite;
        src += slabToWrite;
        size -= slabToWrite;
    } while(size > 0);
    return true;
}
