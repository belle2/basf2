/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov, Mikhail Remnev                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// ECL
#include <ecl/utility/ECLDspUtilities.h>
#include <ecl/utility/ECLChannelMapper.h>
#include <ecl/dbobjects/ECLDspData.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/dataobjects/ECLDigit.h>
// Framework
#include <framework/logging/Logger.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBArray.h>
#include <framework/utilities/FileSystem.h>
// ROOT
#include <TFile.h>
#include <TTree.h>

using namespace Belle2;
using namespace ECL;

/** Version of ECL DSP file format */
const short ECLDSP_FORMAT_VERSION = 1;

int ECLDspUtilities::pedestal_fit_initialized = 0;
float ECLDspUtilities::fg31[768] = {};
float ECLDspUtilities::fg32[768] = {};

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

ECLDspData* ECLDspUtilities::readEclDsp(const char* filename, int boardNumber)
{
  FILE* fl;
  fl = fopen(filename, "rb");
  if (fl == nullptr) {
    B2ERROR("Can't open file " << filename);
  }

  ECLDspData* data = new ECLDspData(boardNumber);

  //== Do not fill data for non-existent shapers
  int crateNum = (boardNumber - 1) / 12 + 1;
  int shaperNum = (boardNumber - 1) % 12;

  if (shaperNum >= 8) {
    if (crateNum >= 45) {
      fclose(fl);
      return data;
    }
    if (shaperNum >= 10) {
      if (crateNum >= 37 && crateNum <= 44) {
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

void ECLDspUtilities::writeEclDsp(const char* filename, ECLDspData* data)
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

/**********************************************/
/*                SHAPE FITTER                */

short int* vectorsplit(std::vector<short int>& vectorFrom, int channel)
{
  size_t size = vectorFrom.size();
  if (size % 16) B2ERROR("Split is impossible!" << "Vector size" << size);
  return (vectorFrom.data() + (size / 16) * (channel - 1));
}

ECLShapeFit ECLDspUtilities::shapeFitter(int cid, std::vector<int> adc, int ttrig,
                                         bool adjusted_timing)
{
  ECLChannelMapper mapper;
  mapper.initFromDB();

  //== Get mapping data

  int crate   = mapper.getCrateID(cid);
  int shaper  = mapper.getShaperPosition(cid);
  int channel = mapper.getShaperChannel(cid);

  //== Get DSP data

  int dsp_group = (crate - 1) / 18;
  int dsp_id = (crate - 1) % 18;
  std::string payload_name = "ECLDSPPars" + std::to_string(dsp_group);
  DBArray<ECLDspData> dsp_data(payload_name);
  const ECLDspData* data = dsp_data[dsp_id * 12 + shaper - 1];

  std::vector<short> vec_f;    data->getF(vec_f);
  std::vector<short> vec_f1;   data->getF1(vec_f1);
  std::vector<short> vec_fg31; data->getF31(vec_fg31);
  std::vector<short> vec_fg32; data->getF32(vec_fg32);
  std::vector<short> vec_fg33; data->getF33(vec_fg33);
  std::vector<short> vec_fg41; data->getF41(vec_fg41);
  std::vector<short> vec_fg43; data->getF43(vec_fg43);

  short* f    = vectorsplit(vec_f,    channel);
  short* f1   = vectorsplit(vec_f1,   channel);
  short* fg31 = vectorsplit(vec_fg31, channel);
  short* fg32 = vectorsplit(vec_fg32, channel);
  short* fg33 = vectorsplit(vec_fg33, channel);
  short* fg41 = vectorsplit(vec_fg41, channel);
  short* fg43 = vectorsplit(vec_fg43, channel);

  int k_a = data->getka();
  int k_b = data->getkb();
  int k_c = data->getkc();
  int k_1 = data->getk1();
  int k_2 = data->getk2();
  int k_16 = data->gety0Startr();
  int chi_thres = data->getchiThresh();

  //== Get ADC data
  int* y = adc.data();

  //== Get trigger time
  int ttrig2 = ttrig - 2 * (ttrig / 8);

  //== Get thresholds
  DBObjPtr<ECLCrystalCalib> thr_LowAmp("ECL_FPGA_LowAmp");
  DBObjPtr<ECLCrystalCalib> thr_HitThresh("ECL_FPGA_HitThresh");
  DBObjPtr<ECLCrystalCalib> thr_StoreDigit("ECL_FPGA_StoreDigit");

  int A0 = thr_LowAmp->getCalibVector()[cid - 1];
  int Ahard = thr_HitThresh->getCalibVector()[cid - 1];
  int Askip = thr_StoreDigit->getCalibVector()[cid - 1];

  //== Perform fit
  auto result = lftda_(f, f1, fg41, fg43, fg31, fg32, fg33, y, ttrig2, A0,
                       Ahard, Askip, k_a, k_b, k_c, k_16, k_1, k_2,
                       chi_thres, adjusted_timing);

  return result;
}

void ECLDspUtilities::initPedestalFit()
{
  std::string path = FileSystem::findFile("ecl/data/ecl_pedestal_peak_fit.root");
  TFile* file = new TFile(path.c_str(), "read");
  TTree* tree = (TTree*)file->Get("dsp_coefs");
  int nentries = tree->GetEntries();
  float fg31_i, fg32_i;
  tree->SetBranchAddress("fg31", &fg31_i);
  tree->SetBranchAddress("fg32", &fg32_i);
  //== Load DSP coefficients used in pedestal fitting
  for (int i = 0; i < nentries; i++) {
    tree->GetEntry(i);
    fg31[i] = fg31_i;
    fg32[i] = fg32_i;
  }
  file->Close();

  pedestal_fit_initialized = 1;
}

ECLPedestalFit ECLDspUtilities::pedestalFit(std::vector<int> adc)
{
  if (!pedestal_fit_initialized) {
    initPedestalFit();
  }

  float amp, tim;

  //== Find maximum in the pedestal

  int ped_max = 0;
  int ped_max_pos = 0;
  for (int i = 0; i < 16; i++) {
    if (adc[i] > ped_max) {
      ped_max = adc[i];
      ped_max_pos = i;
    }
  }

  //== Get first position estimate from maximum location

  int time_index = ped_max_pos * 4 - 8;
  if (time_index > 47) time_index = 47;
  if (time_index < 0)  time_index = 0;

  //== Run two iterations of chi2 minimization
  for (int iter = 0; iter < 2; iter++) {
    amp = 0;
    tim = 0;
    for (int j = 0; j < 16; j++) {
      amp += fg31[j + time_index * 16] * adc[j];
      tim += fg32[j + time_index * 16] * adc[j];
    }
    tim = tim / amp;
    time_index -= tim * 4;
    // time_index -= (tim / amp) * 4;
    if (time_index > 47) time_index = 47;
    if (time_index < 0)  time_index = 0;
  }
  // Estimate time from 0th sample to peak (in denominated microseconds)
  // (1 denom. microsecond = 0.56594/0.5 us)
  tim = time_index * 0.5 - tim + 0.5;

  ECLPedestalFit result;
  result.amp = amp;

  return result;
}

