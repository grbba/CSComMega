
#ifndef Pool_h
#define Pool_h

#include <bit.h>

template<typename T, int S>
class Pool {
private:
    T pool[S];
    T nil;
    int16_t use = 0;    // int 16 i.e. no more than 16 elements in the pool 
                        // every bit resperesnts if one entry is used or not 
    // bool use[S] = {false};
    
    T* _iAllocate() {
        for(int idx = 0; idx < S; idx++ ) {
            if (!checkBit(use, idx)) {
            // if ( use[idx] == false ) {
                // use[idx] = true;
                setBit(use, idx);
                // std::cout << "Allocated " << std::hex << &pool[idx] << std::endl;
                return &pool[idx];
            }
        }
        // no element in the pool is available
        return &nil;
    }

public: 
    
    T* allocate() {
        return _iAllocate();
    };

    void release(T *obj) {
        for(int idx = 0; idx < S; idx++ ) {
            if ( &pool[idx] == obj ) {
                unsetBit(use, idx);
                memset(obj, 0, sizeof(T));
            }
        }
    };

    bool isNil(T*obj) {
        return (obj == &nil);
    }
    
};

#endif