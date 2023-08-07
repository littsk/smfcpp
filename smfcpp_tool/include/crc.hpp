#ifndef CRC_HPP
#define CRC_HPP

#include <iostream>

namespace CRC{

    /*example Mod<uint16_t> means crc16*/
    template<class T>
    struct Mod{
        T Poly, Initvalue, OutputXOR;
        bool InputReverse, OutputReverse;
        Mod(T Poly, T Initvalue, T OutputXOR, bool InputReverse, bool OutputReverse){
            this->Poly = Poly;
            this->Initvalue = Initvalue;
            this->OutputXOR = OutputXOR;
            this->InputReverse = InputReverse;
            this->OutputReverse = OutputReverse;
        }
    };

    template<class T>
    T reverse(T data){
        T ans = 0;
        int len = sizeof(T) * 8;
        for(int i = 0; i < len; ++i){
            ans |= ((data >> i) & 0x0001) << (len - i - 1);
        }
        return ans;
    }

    template<class T>
    void crc_comlete(uint8_t * data, int n, Mod<T> mod){
        uint8_t byte;
        int type_len = sizeof(T);
        T crc = mod.Initvalue;
        for(int i = 0; i < n; ++i){
            byte = *(data + i);
            if(mod.InputReverse){
                byte = reverse(byte);
            }
            crc ^= byte << 8;
            for(int j = 0; j < 8; ++j){
                if(crc & 0x8000){
                    crc = (crc << 1) ^ 0x8005;
                }
                else{
                    crc <<= 1;
                }
            }
        }
        if(mod.OutputReverse){
            crc = reverse(crc);
        }

        for(int i = 0; i < type_len; ++i){
            data[n + i] = crc >> (8 * i);
        }
        
    }
}

#endif

