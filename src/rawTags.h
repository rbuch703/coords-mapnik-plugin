
#ifndef RAW_TAGS_H
#define RAW_TAGS_H

#include <map>
#include <string.h> //for strlen
#include "symbolicNames.h"

class RawTags 
{
public:
    RawTags(uint64_t numTags, uint64_t numTagBytes, 
            const uint8_t *symbolicNameBits, 
            const uint8_t* tagsStart):
        numTags(numTags), numTagBytes(numTagBytes), 
        symbolicNameBits(symbolicNameBits), tagsStart(tagsStart) 
    {}
    
    class RawTagIterator {
    public:
        RawTagIterator(const uint8_t* symbolicNameBits, const uint8_t *tagsAtPos, uint64_t pos):
            symbolicNameBits(symbolicNameBits), tagsAtPos(tagsAtPos), pos(pos)
        { }
        
        bool operator!=(const RawTagIterator &other)
        {
            return tagsAtPos != other.tagsAtPos;
        }
        
        RawTagIterator& operator++()
        {
            tagsAtPos += (isSymbolicName(pos*2)  ? 1 : strlen( (char*)tagsAtPos) + 1);
            tagsAtPos += (isSymbolicName(pos*2+1)? 1 : strlen( (char*)tagsAtPos) + 1);
            pos += 1;
            
            return *this;
        }
        
        std::pair<const char*, const char*> operator*()
        {
            const char* key;
            const char* value;
            
            key = isSymbolicName(pos*2) ? symbolicNames[ *tagsAtPos ] : (char*)tagsAtPos;
            const uint8_t *valuePos = tagsAtPos + 
                (isSymbolicName(pos*2) ? 1 : strlen((char*)tagsAtPos) + 1);
            
            value = isSymbolicName(pos*2+1) ? symbolicNames[ *valuePos ] : (char*)valuePos;
            
            return std::make_pair(key, value);
        }
        
        bool isSymbolicName( int idx) const
        {
            int byteIdx = idx / 8;
            int bitIdx  = idx % 8;
            
            return symbolicNameBits[byteIdx] & (1 << (7 - bitIdx));
        }

    private:
        const uint8_t *symbolicNameBits;
        const uint8_t *tagsAtPos;
        uint64_t pos;
    };
    
    RawTagIterator begin() const { return RawTagIterator(symbolicNameBits, tagsStart, 0);}
    RawTagIterator end()   const { return RawTagIterator(symbolicNameBits, tagsStart + numTagBytes, numTags);}
    
private:
    uint64_t numTags;
    uint64_t numTagBytes;
    const uint8_t *symbolicNameBits;
    const uint8_t *tagsStart;
};

#endif
