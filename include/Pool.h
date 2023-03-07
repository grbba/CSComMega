
#ifndef Pool_h
#define Pool_h

#include <bit.h>

/*
message class is for testing purposes only 
*/
class message {
public:
    int a = 0;
    char c = '\0';
    message(){};
    message(int v1, char v2)
    {
        a = v1; 
        c = v2;
    }; 

    // void reset() { 
    //     a = 0;
    //     c = '\0';
    // };

};


template<typename T, int S>
class Pool {
private:
    T pool[S];
    int16_t use = 0;    // int 16 i.e. no more than 16 elements in the pool 
                        // every bit resperesnts if one entry is used or not 
    // bool use[S] = {false};
    
    T* _iAllocate() {
        for(int idx = 0; idx < S; idx++ ) {
            if (!checkBit(use, idx)) {
            // if ( use[idx] == false ) {
                // use[idx] = true;
                setBit(use, idx);
                return &pool[idx];
            }
        }
        // no element in the pool is available
        return nullptr;
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
};

#endif