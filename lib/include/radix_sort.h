#if !defined(RADIX_SORT_HEADER)
#define RADIX_SORT_HEADER

/* Radix sort for positive integers 

Example for sorting integer vectors via y coordinate:

```
uint32_t PointSortIndex(void *point) {
    Vec2i *p = (Vec2i *point);
    return p->y;
}

// ...

Vec2i points[4] = { (Vec2i){0, 2}, (Vec2i){3, 8}, (Vec2i){4, 3}, (Vec2i){5, 1}, };

RadixSort(points, sizeof(points[0]), 4, PointSortIndex);
```
*/

typedef unsigned int SortIndexProc(void *);
void RadixSort(void *base, size_t elementSize, size_t count, SortIndexProc *sortIndexProc);
void RadixSortReverse(void *base, size_t elementSize, size_t count, SortIndexProc *sortIndexProc);

// Example:

#endif // RADIX_SORT_HEADER

#if defined(RADIX_SORT_IMPLEMENTATION)
#if !defined(RADIX_SORT_IMPLEMENTED)
#define RADIX_SORT_IMPLEMENTED


void RadixSort(void *base, size_t elementSize, size_t count, SortIndexProc sortIndexProc) {
    // For now I'm using the stable version (using extra memory for the sort), but there's also an in-place version.
    static void *temp = NULL;
    arrsetcap(temp, count * elementSize);
    arrsetlen(temp, 0);
    
    void *source = base;
    void *dest   = temp;
    
    // Sorting using four passes of 8-bit indices
    for (int byteIndex = 0; byteIndex < 4; ++byteIndex) {
        size_t sortOffsets[256] = {}; // 2^8-bit buckets
        
#define RADIX_INDEX(element) ((sortIndexProc(element) >> (byteIndex*8)) & 0xFF)
        
        // First pass - counting edges for each bucket
        for (int i = 0; i < count; ++i) {
            void *element = source + i * elementSize;
            sortOffsets[RADIX_INDEX(element)] += 1;
        }
        
        // Convert the counts into indices
        int currentOffset = 0;
        for (int i = 0; i < sizeof(sortOffsets)/sizeof(sortOffsets[0]); ++i) {
            int bucketSize = sortOffsets[i];
            sortOffsets[i] = currentOffset;
            currentOffset += bucketSize;
        }
        
        // Second pass - sort the elements using the current byte part
        for (int i = 0; i < count; ++i) {
            void *element = source + i * elementSize;
            size_t elementIndex = sortOffsets[RADIX_INDEX(element)];
            memcpy(dest + elementIndex * elementSize, element, elementSize);
            sortOffsets[RADIX_INDEX(element)] += 1;
        }
#undef RADIX_INDEX
        
        // Swap source and dest for the next iteration
        void *swap = dest;
        dest = source;
        source = swap;
    }
}

void RadixSortReverse(void *base, size_t elementSize, size_t count, SortIndexProc sortIndexProc) {
    // For now I'm using the stable version (using extra memory for the sort), but there's also an in-place version.
    static void *temp = NULL;
    arrsetcap(temp, count * elementSize);
    arrsetlen(temp, 0);
    
    void *source = base;
    void *dest   = temp;
    
    // Sorting using four passes of 8-bit indices
    for (int byteIndex = 0; byteIndex < 4; ++byteIndex) {
        size_t sortOffsets[256] = {}; // 2^8-bit buckets
        
#define RADIX_INDEX(element) (255 - ((sortIndexProc(element) >> (byteIndex*8)) & 0xFF))
        
        // First pass - counting edges for each bucket
        for (int i = 0; i < count; ++i) {
            void *element = source + i * elementSize;
            sortOffsets[RADIX_INDEX(element)] += 1;
        }
        
        // Convert the counts into indices
        int currentOffset = 0;
        for (int i = 0; i < sizeof(sortOffsets)/sizeof(sortOffsets[0]); ++i) {
            int bucketSize = sortOffsets[i];
            sortOffsets[i] = currentOffset;
            currentOffset += bucketSize;
        }
        
        // Second pass - sort the elements using the current byte part
        for (int i = 0; i < count; ++i) {
            void *element = source + i * elementSize;
            size_t elementIndex = sortOffsets[RADIX_INDEX(element)];
            memcpy(dest + elementIndex * elementSize, element, elementSize);
            sortOffsets[RADIX_INDEX(element)] += 1;
        }
#undef RADIX_INDEX
        
        // Swap source and dest for the next iteration
        void *swap = dest;
        dest = source;
        source = swap;
    }
}

#endif // RADIX_SORT_IMPLEMENTED
#endif // RADIX_SORT_IMPLEMENTATION
