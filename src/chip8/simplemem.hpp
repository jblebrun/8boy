#include "memory.hpp"

class SimpleMemory : public Memory {
    static const uint16_t SIZE = 8*1024;
    uint8_t mMemory[SIZE];

    public:
    SimpleMemory();
    virtual void load(const uint8_t *program, const uint16_t size);
    virtual bool read(uint16_t addr, uint8_t *dest, uint8_t size);
    virtual bool write(uint16_t addr, uint8_t *src, uint8_t size);
    virtual void reset() {};
};
