//
// Created by sebastian on 24.11.20.
//

#ifndef DBVH_ALIGNEDALLOCATOR_H
#define DBVH_ALIGNEDALLOCATOR_H

#include <iostream>
#include <cassert>

#define ALIGNMENT 64

void *aligned_malloc(size_t align, size_t size);

void aligned_free(void *ptr);

// Convenience macro for memalign, the POSIX API
#define memalign(align, size) aligned_malloc(align, size)

// Number of bytes we're using for storing
// the aligned pointer offset
typedef uint16_t offset_t;
#define PTR_OFFSET_SZ sizeof(offset_t)

#ifndef align_up
#define align_up(num, align) \
    (((num) + ((align) - 1)) & ~((align) - 1))
#endif

void *aligned_malloc(uint64_t align, uint64_t size) {
    void *ptr = NULL;

    // We want it to be a power of two since
    // align_up operates on powers of two
    assert((align & (align - 1)) == 0);

    if (align && size) {
        /*
         * We know we have to fit an offset value
         * We also allocate extra bytes to ensure we
         * can meet the alignment
         */
        uint32_t hdr_size = PTR_OFFSET_SZ + (align - 1);
        void *p = malloc(size + hdr_size);

        if (p) {
            /*
             * Add the offset size to malloc's pointer
             * (we will always store that)
             * Then align the resulting value to the
             * target alignment
             */
            ptr = (void *) align_up(((uintptr_t) p + PTR_OFFSET_SZ), align);

            // Calculate the offset and store it
            // behind our aligned pointer
            *((offset_t *) ptr - 1) =
                    (offset_t) ((uintptr_t) ptr - (uintptr_t) p);

        } // else NULL, could not malloc
    } //else NULL, invalid arguments
    return ptr;
}

void aligned_free(void *ptr) {
    assert(ptr);

    /*
    * Walk backwards from the passed-in pointer
    * to get the pointer offset. We convert to an offset_t
    * pointer and rely on pointer math to get the data
    */
    offset_t offset = *((offset_t *) ptr - 1);

    /*
    * Once we have the offset, we can get our
    * original pointer and call free
    */
    void *p = (void *) ((uint8_t *) ptr - offset);
    free(p);
}

template<typename T>
class AlignedWrapper {
public:
    T data;

private:
    uint8_t padding[ALIGNMENT - (sizeof(T) % ALIGNMENT)];

public:

    static void *operator new(size_t size) {
        return memalign(ALIGNMENT, size);
    }

    static void *operator new[](size_t size) {
        return memalign(ALIGNMENT, size);
    }

    static void operator delete(void *ptr) _GLIBCXX_USE_NOEXCEPT {
        aligned_free(ptr);
    }

    static void operator delete[](void *ptr) _GLIBCXX_USE_NOEXCEPT {
        aligned_free(ptr);
    }

    AlignedWrapper() = default;

    template<class ...Args>
    AlignedWrapper(Args... args) : data(args...) {

    }
};

#endif //DBVH_ALIGNEDALLOCATOR_H
