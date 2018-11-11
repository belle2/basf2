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
    /** Version of ECL DSP file format */
    const short ECLDSP_FORMAT_VERSION = 1;

    /**
     * @brief Read data from file to ptr.
     * @return Number of times sucessfully read.
     */
    void readEclDspCoefs(FILE* fl, void* ptr, int word_size, int word_count)
    {
      int read_items = fread(ptr, word_size, word_count, fl);
      if (read_items != word_count) {
        B2ERROR("Error reading DSP coefficients");
      }
    }

    ECLDspData* readEclDsp(const char* filename, int boardNumber)
    {
      FILE* fl;
      fl = fopen(filename, "rb");
      if (fl == NULL) {
        B2ERROR("Can't open file " << filename);
      }

      ECLDspData* data = new ECLDspData(boardNumber);

      //== Do not fill data for non-existent shapers
      std::string crate_id_str = std::string(filename - 12 + strlen(filename)).substr(0, 2);
      int crate_id = std::stoi(crate_id_str);
      if (boardNumber > 7) {
        if (crate_id >= 45 && crate_id <= 52) {
          fclose(fl);
          return data;
        }
        if (boardNumber > 9) {
          if (crate_id >= 37 && crate_id <= 44) {
            fclose(fl);
            return data;
          }
        }
      }

      // Word size
      const int nsiz = 2;
      // Size of fragment to read (in words)
      int nsiz1 = 256;
      short int id[256];
      int size = fread(id, nsiz, nsiz1, fl);
      if (size != nsiz1) {
        B2ERROR("Error reading header of DSP file");
      }

      std::vector<short int> extraData;
      extraData.push_back(ECLDSP_FORMAT_VERSION);
      extraData.push_back(data->getPackerVersion());
      data->setExtraData(extraData);

      data->setverMaj(id[8] & 0xFF);
      data->setverMin(id[8] >> 8);
      data->setkb(id[13] >> 8);
      data->setka(id[13] - 256 * data->getkb());
      data->sety0Startr(id[14] >> 8);
      data->setkc(id[14] - 256 * data->gety0Startr());
      data->setchiThresh(id[15]);
      data->setk2(id[16] >> 8);
      data->setk1(id[16] - 256 * data->getk2());
      data->sethT(id[64]);
      data->setlAT(id[128]);
      data->setsT(id[192]);
      data->setaAT(id[208]);

      std::vector<short int> f(49152), f1(49152), f31(49152),
          f32(49152), f33(49152), f41(6144), f43(6144);

      for (int i = 0; i < 16; ++i) {
        nsiz1 = 384;
        readEclDspCoefs(fl, &(*(f41.begin() + i * nsiz1)), nsiz, nsiz1);
        nsiz1 = 3072;
        readEclDspCoefs(fl, &(*(f31.begin() + i * nsiz1)), nsiz, nsiz1);
        readEclDspCoefs(fl, &(*(f32.begin() + i * nsiz1)), nsiz, nsiz1);
        readEclDspCoefs(fl, &(*(f33.begin() + i * nsiz1)), nsiz, nsiz1);
        nsiz1 = 384;
        readEclDspCoefs(fl, &(*(f43.begin() + i * nsiz1)), nsiz, nsiz1);
        nsiz1 = 3072;
        readEclDspCoefs(fl, &(*(f.begin() + i * nsiz1)), nsiz, nsiz1);
        readEclDspCoefs(fl, &(*(f1.begin() + i * nsiz1)), nsiz, nsiz1);
      }
      fclose(fl);

      data->setF41(f41);
      data->setF31(f31);
      data->setF32(f32);
      data->setF33(f33);
      data->setF43(f43);
      data->setF(f);
      data->setF1(f1);

      return data;
    }

    void writeEclDsp(const char* filename, ECLDspData* data)
    {
      // Default header for DSP file.
      // Words '0xABCD' are overwitten with current parameters.
      const unsigned short DEFAULT_HEADER[256] {
        // ECLDSP FILE.....
        0x4543, 0x4c44, 0x5350, 0x2046, 0x494c, 0x4500, 0x0000, 0x0000,
        0xABCD, 0xffff, 0x0000, 0x0000, 0x0000, 0xABCD, 0xABCD, 0xABCD,
        0xABCD, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD,
        0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD,
        0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD,
        0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD,
        0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD,
        0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD, 0xABCD,
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
      unsigned short header[256];

      int format_version = data->getExtraData()[0];
      if (format_version > ECLDSP_FORMAT_VERSION) {
        B2WARNING("Format version " << format_version << " is not fully supported, some data might be discarded.");
      }

      for (int i = 0; i < 256; i++) {
        if (i == 8) {
          header[i] = (data->getverMin() << 8) | data->getverMaj();
        } else if (i == 13) {
          header[i] = (data->getkb() << 8) | data->getka();
        } else if (i == 14) {
          header[i] = (data->gety0Startr() << 8) | data->getkc();
        } else if (i == 15) {
          header[i] = data->getchiThresh();
        } else if (i == 16) {
          header[i] = (data->getk2() << 8) | data->getk1();
        } else if (i >= 64 && i < 80) {
          header[i] = data->gethT();
        } else if (i >= 128 && i < 144) {
          header[i] = data->getlAT();
        } else if (i >= 192 && i < 208) {
          header[i] = data->getsT();
        } else if (i >= 208 && i < 224) {
          header[i] = data->getaAT();
        } else {
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

      std::vector<short int> f(49152), f1(49152), f31(49152),
          f32(49152), f33(49152), f41(6144), f43(6144);
      data->getF41(f41);
      data->getF31(f31);
      data->getF32(f32);
      data->getF33(f33);
      data->getF43(f43);
      data->getF(f);
      data->getF1(f1);

      for (int i = 0; i < 16; ++i) {
        nsiz1 = 384;
        fwrite(&(*(f41.begin() + i * nsiz1)), nsiz, nsiz1, fl);
        nsiz1 = 3072;
        fwrite(&(*(f31.begin() + i * nsiz1)), nsiz, nsiz1, fl);
        fwrite(&(*(f32.begin() + i * nsiz1)), nsiz, nsiz1, fl);
        fwrite(&(*(f33.begin() + i * nsiz1)), nsiz, nsiz1, fl);
        nsiz1 = 384;
        fwrite(&(*(f43.begin() + i * nsiz1)), nsiz, nsiz1, fl);
        nsiz1 = 3072;
        fwrite(&(*(f.begin() + i * nsiz1)), nsiz, nsiz1, fl);
        fwrite(&(*(f1.begin() + i * nsiz1)), nsiz, nsiz1, fl);
      }
      fclose(fl);
    }
  }
}

