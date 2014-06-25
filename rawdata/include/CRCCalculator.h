#ifndef CRCCALCULATOR_H
#define CRCCALCULATOR_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <cstddef>


//namespace Belle2 {
/**
 * Function to calculate CRC16
 * Belle2link(HSLB/FEE) trailers.are supposed to contain CRC information.
 * This function will be used to check them in DAQ system.
 */
unsigned short CalcCRC16(unsigned short crc16, const char buf[], size_t size);

unsigned short CalcCRC16LittleEndian(unsigned short crc16, const int buf[], int nwords);

/**
 * Function to copy data
 * Just copy data from buf_from to (buf_to + pos_nwords_to)
 * It also check buffer overflow by using nwords_buf_to
 */
void copyData(int* buf_to, int* pos_nwords_to, const int* buf_from,
              const int copy_nwords, const int nwords_buf_to);



//}
#endif
