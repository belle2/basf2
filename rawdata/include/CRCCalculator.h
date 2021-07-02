/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef CRCCALCULATOR_H
#define CRCCALCULATOR_H

//namespace Belle2 {

/**
 * Function to calculate CRC16 (polynomial 0x1021, initial value=0xffff)
 * Belle2link(HSLB/FEE) trailers.are supposed to contain CRC information.
 * This function will be used to check them in DAQ system if CPU power allows.
 */
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
