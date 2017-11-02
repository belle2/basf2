/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tzu-An Sheng                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "trg/cdc/modules/houghtracking/xsi_loader.h"

#include <trg/cdc/modules/trgcdc//CDCTriggerTSFFirmwareModule.h>
#include <framework/datastore/StoreArray.h>
#include <cdc/dataobjects/CDCHit.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>

#include <string>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <vector>
#include <array>
#include <algorithm>
#include <numeric>
#include <bitset>
#include <experimental/array>
#include <math.h>

using std::experimental::to_array;
constexpr double pi() { return std::atan(1) * 4; }



/**
 * Helper class for software (C++) / firmware (VHDL) co-simulation
 */
namespace Cosim {
  /* Note: VHDL std_logic value is stored in a byte (char). The
   * 9 values are mapped as  'U':0, 'X':1, '0':2, '1':3
   * 'Z':4, 'W':5, 'L':6, 'H':7, '-':8 . The std_logic_vector
   * is stored as a contiguous array of bytes. For example
   * "0101Z" is stored in five bytes as char s[5] = {2,3,2,3,4}
   * An HDL integer type is stored as C int, a HDL real type is
   * stored as a C double and a VHDL string type is stored as char*.
   * An array of HDL integer or double is stored as an array of C
   * integer or double respectively
   */

  /* In case you are not familiar with VHDL simulation, there are 9 possible
   * values defined for the standard logic type, instead of just 0 and 1. The
   * simulator needs to compute all these possible outcomes. Therefore, XSI uses
   * a byte, instead of a bit, to represent a std_logic. This is represented
   * with a char with possible values ranging from 0 to 8.
   */
  const char* std_logic_literal[] = {"U", "X", "0", "1", "Z", "W", "L", "H", "-"};

  /** '1' in XSI VHDL simulation */
  const char one_val  = 3;
  /** '0' in XSI VHDL simulation */
  const char zero_val = 2;

  std::string display_value(const char* count, int size)
  {
    std::string res;
    for (int i = 0; i < size; i++)
      res += std_logic_literal[(int) count[i]];
    return res;
  }

  template<size_t N>
  std::string slv_to_bin_string(std::array<char, N> signal, bool padding = false)
  {
    int ini = padding ? signal.size() % 4 : 0;
    std::string res(ini, '0');
    for (auto const& bit : signal) {
      res += std_logic_literal[(int) bit];
    }
    return res;
  }

  template<size_t N>
  void display_hex(const std::array<char, N>& signal)
  {
    std::ios oldState(nullptr);
    oldState.copyfmt(std::cout);
    if (std::any_of(signal.begin(), signal.end(), [](char i)
    {return i != zero_val && i != one_val;})) {
      B2WARNING("Some bit in the signal vector is neither 0 nor 1.");
    }
    std::string binString = slv_to_bin_string(signal, true);
    std::cout << std::setfill('0');
    for (unsigned i = 0; i < signal.size(); i += 4) {
      std::bitset<4> set(binString.substr(i, 4));
      std::cout << std::setw(1) << std::hex << set.to_ulong();
    }
    std::cout << "\n";
    std::cout.copyfmt(oldState);
  }


  /* extract a subset of bitstring, like substring.
   *
   * In principle this can be done using only integer manipulations, but for the
   * sake of simplicity, let's just cast them to string. Beware the endianness.
   * 0 refer to the rightmost bit in std::bitset, but the leftmost bit in
   * std::string
   */
  template<size_t nbits, size_t min, size_t max>
  std::bitset < max - min + 1 > subset(std::bitset<nbits> set)
  {
    const size_t outWidth = max - min + 1;
    std::string str = set.to_string();
    return std::bitset<outWidth>(str.substr(nbits - max - 1, outWidth));
  }
}

int Xsi::Loader::get_port_number_or_exit(std::string name)
{
  int portNumber = get_port_number(name.c_str());
  if (portNumber < 0) {
    std::cerr << "ERROR: " << name << " not found" << std::endl;
    throw;
  }
  return portNumber;
}


using namespace Belle2;
using namespace Cosim;
using namespace std;

REG_MODULE(CDCTriggerTSFFirmware)


constexpr std::array<int, Belle2::CDCTriggerTSFFirmwareModule::m_nSubModules>
Belle2::CDCTriggerTSFFirmwareModule::nMergers;

CDCTriggerTSFFirmwareModule::CDCTriggerTSFFirmwareModule() :
  Module(), m_cdcHits("")

{
  for (unsigned i = 0; i < m_nSubModules; ++i) {
    // if (i == 2) continue;
    // loader.emplace_back(new Xsi::Loader(design_libname_pre + to_string(i * 2)
    //                                     + design_libname_post, simengine_libname));
    inputToTSF[i].resize(nMergers[i]);
  }
  //Set module properties
  setDescription("Firmware simulation of the Track Segment Finder for CDC trigger.");

  // Define module parameters
  addParam("hitCollectionName", m_hitCollectionName,
           "Name of the input StoreArray of CDCHits.",
           string(""));
  // addParam("outputCollectionName", m_outputCollectionName,
  //          "Name of the StoreArray holding the tracks found in the Hough tracking.",
  //          string("TRGCDCTSFFirmwareTracks"));
}

CDCTriggerTSFFirmwareModule::~CDCTriggerTSFFirmwareModule()
{
  for (auto& l : loader) {
    l->close();
  }
}

void CDCTriggerTSFFirmwareModule::initialize()
{
  m_cdcHits.isRequired(m_hitCollectionName);

  std::cout << "Design DLL     : " << design_libname_pre << "*" <<
            design_libname_post << std::endl;
  std::cout << "Sim Engine DLL : " << simengine_libname << std::endl;

  // try {
  array<string, m_nSubModules> wdbName;
  for (unsigned i = 0; i < m_nSubModules; ++i) {
    // if (i==1) continue;
    if (i < 2) continue;
    memset(&info[i], 0, sizeof(info[i]));
    // char logName[] = "firmware.log";
    // info[i].logFileName = logName;
    info[i].logFileName = NULL;
    // char wdbName[] = "tsf" + to_string(i * 2) + ".wdb";
    // string wdbName = "tsf" + to_string(i * 2) + ".wdb";
    wdbName[i] = "tsf" + to_string(i * 2) + ".wdb";
    info[i].wdbFileName = const_cast<char*>(wdbName[i].c_str());
    // Xsi::Loader thisloader(design_libname_pre + to_string(i * 2)
    //                        + design_libname_post, simengine_libname);
    Xsi::Loader thisloader("/home/belle2/tasheng/tsim/xsim.dir/tsf4/xsimk.so",
                           simengine_libname);
    cout << "about to open tsf" << i * 2 << "\n";
    s_xsi_setup_info thisinfo;
    memset(&thisinfo, 0, sizeof(thisinfo));
    thisinfo.logFileName = NULL;
    thisinfo.wdbFileName = const_cast<char*>(wdbName[i].c_str());

    cout << design_libname_pre + to_string(i * 2)
         + design_libname_post << ", " << simengine_libname << "\n";
    // loader[i]->open(&thisinfo);
    thisloader.open(&thisinfo);
    cout << "loader opened" << "\n";
    loader[i]->trace_all();
    cout << "traced" << "\n";
    clk[i]  = loader[i]->get_port_number_or_exit("user_clk_127");
    cout << "clock get" << "\n";
  }

  for (int iSL = 0; iSL < m_nSubModules; ++iSL) {
    if (iSL == 1) continue;
    string inName = "data_in";
    string outName = "TSF_TRACKER_OUT";
    string enableInName = "valid_in";
    string enableTrackerName = "valid_tracker";
    string enableEVTName = "valid_evttime";
    inPort[iSL] = loader[iSL]->get_port_number_or_exit(inName);
    outPort[iSL] = loader[iSL]->get_port_number_or_exit(outName);
    enableIn[iSL] = loader[iSL]->get_port_number_or_exit(enableInName);
    enableOutTracker[iSL] = loader[iSL]->get_port_number_or_exit(enableTrackerName);
    enableOutEVT[iSL] = loader[iSL]->get_port_number_or_exit(enableEVTName);
    // Start low clock
    loader[iSL]->put_value(clk[iSL], &zero_val);
    cout << "ready to run" << "\n";
    loader[iSL]->run(10);
  }

  // } catch (std::exception& e) {
  //   std::cerr << "ERROR: An exception occurred: " << e.what() << std::endl;
  // } catch (...) {
  //   std::cerr << "ERROR: An unknown exception occurred." << std::endl;
  //   // Xsi_Instance.get_error_info();
  //   throw;
  // }
}

template<int iSL>
char* CDCTriggerTSFFirmwareModule::getData(inputToTSFArray input)
{
  static array<char, width_in* nMergers[iSL]> data;
  auto itr = data.begin();
  for (const auto& merger : input[iSL]) {
    copy(merger.begin(), merger.end(), itr);
    itr += width_in;
  }
  return data.data();
}

int CDCTriggerTSFFirmwareModule::localSegmentID(int globalID)
{
  std::array<int, 9> nWiresPerLayer = {160, 160, 192, 224, 256, 288, 320, 352, 384};
  // std::array<int, 8> nFEPerLayer = {5, 5, 6, 7, 8, 9, 10, 11, 12};
  std::array<int, 10> nAccumulate;
  std::partial_sum(nWiresPerLayer.begin(), nWiresPerLayer.end(), nAccumulate.begin() + 1);
  // shift the ID by 16 in SL8
  if (globalID > nAccumulate[7]) {
    globalID += 16;
  }
  auto nWiresInside = std::lower_bound(nAccumulate.begin(), nAccumulate.end(), globalID) - 1;
  return globalID - *nWiresInside;
}

// auto CDCTriggerTSFFirmwareModule::toTSSLV(const std::bitset<m_tsVectorWidth> ts)
// {
//   std::string word = ts.to_string();
//   std::array<char, m_tsVectorWidth> vec;
//   for (unsigned i = 0; i < word.size(); ++i) {
//     vec[i] = (word[i] == '0') ? zero_val : one_val;
//   }
//   return vec;
// }


void CDCTriggerTSFFirmwareModule::event()
{
  int status = 0;
  // data clock period (32ns) in unit of 2ns
  const int clockPeriod = 16;

  // get cdchits from data store
  // assign clock to hits
  const int nAxialSuperLayer = 5;

  std::vector<int> iAxialHit;
  for (int i = 0; i < m_cdcHits.getEntries(); ++i) {
    if (m_cdcHits[i]->getISuperLayer() % 2 == 0) {
      iAxialHit.push_back(i);
    }
  }
  std::sort(iAxialHit.begin(), iAxialHit.end(), [this](int i, int j) {
    return m_cdcHits[i]->getTDCCount() < m_cdcHits[j]->getTDCCount();
  });

  for (auto hit : iAxialHit) {
    B2DEBUG(10, "sorted TDC count: " << m_cdcHits[hit]->getTDCCount());
  }

  // assign input signals
  try {
    for (int iClock = 0; iClock < m_nClockPerEvent; iClock++) {
      std::cout << "------------------" << "\n";
      std::cout << "clock #" << iClock << "\n";
      auto TSF0 = getData<0>(inputToTSF);
      auto TSF2 = getData<1>(inputToTSF);
      auto TSF4 = getData<2>(inputToTSF);
      auto TSF6 = getData<3>(inputToTSF);
      auto TSF8 = getData<4>(inputToTSF);
      array<char*, m_nSubModules> rawInputToTSF = {TSF0, TSF2, TSF4, TSF6, TSF8};

      for (unsigned iSL = 0; iSL < nAxialSuperLayer; ++iSL) {
        if (iSL == 1) continue;
        // Put clk to one
        loader[iSL]->put_value(clk[iSL], &one_val);
        loader[iSL]->run(10);

        // // firstly, clear the input signal vectors
        // tsfInput[iSL].fill(zero_val);
        // // then, copy the TS info to the input signal vectors
        // // unless there is no more hit in the SL
        // auto itr = tsfInput[iSL].begin() + 9;
        // if (iClock < nClocks) {
        //   for (auto ts : tsHitVector[iSL][iClock]) {
        //     tsVector inTS(toTSSLV(ts));
        //     copy(inTS.begin(), inTS.end(), itr);
        //     advance(itr, m_tsVectorWidth);
        //   }
        // }

        cout << "input to TSF" << iSL * 2 << ": ";
        display_value(rawInputToTSF[iSL], nMergers[iSL] * width_in);
        // write the input
        loader[iSL]->put_value(inPort[iSL], rawInputToTSF[iSL]);

        // read the output
        loader[iSL]->get_value(outPort[iSL], outputToTracker[iSL].data());

        // Put clk to zero
        loader[iSL]->put_value(clk[iSL], &zero_val);
        loader[iSL]->run(10);
      }

      // string found = display_value(t2d_out, 6);
      // std::cout << "output: " << found << "\n";
      // for (unsigned j = 0; j < count(found.begin(), found.end(), '1'); j++) {
      //   std::cout << display_value(t2d_out + 6 + 121 * j, 121) << "\n";
      // }
      // finderOutput = to_array(t2d_out);
    }
    std::cout << "status code:" << status << std::endl;
  } catch (std::exception& e) {
    std::cerr << "ERROR: An exception occurred: " << e.what() << std::endl;
    status = 2;
  } catch (...) {
    std::cerr << "ERROR: An unknown exception occurred." << std::endl;
    status = 3;
  }
}
