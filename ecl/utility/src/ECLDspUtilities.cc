/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov, Mikhail Remnev                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// ECL
#include <ecl/utility/ECLDspUtilities.h>
#include <ecl/dbobjects/ECLDspData.h>
// Framework
#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace ECL {
    ECLDspData* readEclDsp(const char* filename, int boardNumber)
    {
      FILE* fl;
      fl = fopen(filename, "rb");
      if (fl == NULL) {
        B2ERROR("Can't open file " << filename);
      }

      ECLDspData* data = new ECLDspData(boardNumber);

      // Word size
      const int nsiz = 2;
      // Size of fragment to read (in words)
      int nsiz1 = 256;
      short int id[256];
      int size = fread(id, nsiz, nsiz1, fl);
      if (size != nsiz1) {
        B2ERROR("Error reading data");
      }
      data->setkb(id[13] >> 8);
      data->setka(id[13] - 256 * data->getkb());
      data->sety0Startr(id[14] >> 8);
      data->setkc(id[14] - 256 * data->gety0Startr());
      data->setchiThresh(id[15]);
      data->setk2(id[16] >> 8);
      data->setk1(id[16] - 256 * data->getk2());
      for (int i = 0; i < 16; ++i) {
        nsiz1 = 384;
        size = fread(&(*(data->getF41().begin() + i * nsiz1)), nsiz, nsiz1, fl);
        nsiz1 = 3072;
        size = fread(&(*(data->getF31().begin() + i * nsiz1)), nsiz, nsiz1, fl);
        size = fread(&(*(data->getF32().begin() + i * nsiz1)), nsiz, nsiz1, fl);
        size = fread(&(*(data->getF33().begin() + i * nsiz1)), nsiz, nsiz1, fl);
        nsiz1 = 384;
        size = fread(&(*(data->getF43().begin() + i * nsiz1)), nsiz, nsiz1, fl);
        nsiz1 = 3072;
        size = fread(&(*(data->getF().begin() + i * nsiz1)), nsiz, nsiz1, fl);
        size = fread(&(*(data->getF1().begin() + i * nsiz1)), nsiz, nsiz1, fl);
      }
      fclose(fl);

      return data;
    }

    void writeEclDsp(const char* filename, ECLDspData* data)
    {
      // Default header for DSP file.
      // Words '0xABCD' are overwitten with current parameters.
      const unsigned short DEFAULT_HEADER[256] {
        // ECLDSP FILE.....
        0x4543, 0x4c44, 0x5350, 0x2046, 0x494c, 0x4500, 0x0000, 0x0000,
        0x0102, 0xffff, 0x0000, 0x0000, 0x0000, 0xABCD, 0xABCD, 0xABCD,
        0xABCD, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x9cff, 0x9cff, 0x9cff, 0x9cff, 0x9cff, 0x9cff, 0x9cff, 0x9cff,
        0x9cff, 0x9cff, 0x9cff, 0x9cff, 0x9cff, 0x9cff, 0x9cff, 0x9cff,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x9e00, 0x9e00, 0x9e00, 0x9e00, 0x9e00, 0x9e00, 0x9e00, 0x9e00,
        0x9e00, 0x9e00, 0x9e00, 0x9e00, 0x9e00, 0x9e00, 0x9e00, 0x9e00,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x6200, 0x6200, 0x6200, 0x6200, 0x6200, 0x6200, 0x6200, 0x6200,
        0x6200, 0x6200, 0x6200, 0x6200, 0x6200, 0x6200, 0x6200, 0x6200,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
      };

      FILE* fl;
      fl = fopen(filename, "wb");
      // Word size
      const int nsiz = 2;
      // Size of fragment to write (in words)
      int nsiz1 = 256;
      // modification of DEFAULT_HEADER
      short header[256];

      for (int i = 0; i < 256; i++) {
        switch (i) {
          case 13:
            header[i] = (data->getkb() << 8) | data->getka();
            break;
          case 14:
            header[i] = (data->gety0Startr() << 8) | data->getkc();
            break;
          case 15:
            header[i] = data->getchiThresh();
            break;
          case 16:
            header[i] = (data->getk2() << 8) | data->getk1();
            break;
          default:
            // Reverse bytes for header.
            int high = (DEFAULT_HEADER[i] & 0xFF00) >> 8;
            int low  = (DEFAULT_HEADER[i] & 0x00FF);
            header[i] = (low << 8) + high;
        }
      }

      int size = fwrite(header, nsiz, nsiz1, fl);
      if (size != nsiz1) {
        B2FATAL("Error writing header of DSP file " << filename);
      }

      for (int i = 0; i < 16; ++i) {
        nsiz1 = 384;
        size = fwrite(&(*(data->getF41().begin() + i * nsiz1)), nsiz, nsiz1, fl);
        nsiz1 = 3072;
        size = fwrite(&(*(data->getF31().begin() + i * nsiz1)), nsiz, nsiz1, fl);
        size = fwrite(&(*(data->getF32().begin() + i * nsiz1)), nsiz, nsiz1, fl);
        size = fwrite(&(*(data->getF33().begin() + i * nsiz1)), nsiz, nsiz1, fl);
        nsiz1 = 384;
        size = fwrite(&(*(data->getF43().begin() + i * nsiz1)), nsiz, nsiz1, fl);
        nsiz1 = 3072;
        size = fwrite(&(*(data->getF().begin() + i * nsiz1)), nsiz, nsiz1, fl);
        size = fwrite(&(*(data->getF1().begin() + i * nsiz1)), nsiz, nsiz1, fl);
      }
      fclose(fl);
    }
  }
}

