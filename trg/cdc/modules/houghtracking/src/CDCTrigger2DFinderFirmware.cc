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

#include <trg/cdc/modules/houghtracking/CDCTrigger2DFinderModule.h>
#include <trg/cdc/modules/houghtracking/CDCTrigger2DFinderFirmware.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>

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
#include <gsl/gsl_const_mksa.h>

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

  // template<size_t N>
  // // void display_hex_from_bitset(std::bitset<N> const & signal)
  // void display_hex(std::bitset<N> const & signal)
  // {
  //   std::cout << "binary: " << signal << "\n";
  //   std::cout << "hex: " << std::hex << signal.to_ulong() << "\n";
  // }

  // template<size_t N>
  // void display_hex(const std::array<char, N> & signal)
  // {
  //   if (std::any_of(signal.begin(), signal.end(), [] (char i) {return i != 2 && i != 3;})) {
  //     B2ERROR("Some bit in the signal vector is neither 0 nor 1.");
  //   }
  //   display_hex(std::bitset<signal.size()>(slv_to_bin_string(signal)));
  // }

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

CDCTrigger2DFinderFirmware::CDCTrigger2DFinderFirmware(const CDCTrigger2DFinderModule* mod) :
  Xsi_Instance(design_libname, simengine_libname),
  m_mod(mod)
{

}

CDCTrigger2DFinderFirmware::~CDCTrigger2DFinderFirmware()
{
  Xsi_Instance.close();
}

void CDCTrigger2DFinderFirmware::initialize()
{
  std::cout << "Design DLL     : " << design_libname << std::endl;
  std::cout << "Sim Engine DLL : " << simengine_libname << std::endl;
  try {
    memset(&info, 0, sizeof(info));
    char logName[] = "firmware.log";
    // info.logFileName = NULL;
    info.logFileName = logName;
    char wdbName[] = "test.wdb";
    info.wdbFileName = wdbName;
    Xsi_Instance.open(&info);
    Xsi_Instance.trace_all();

    clk  = Xsi_Instance.get_port_number_or_exit("Top_clkData_s");
    for (int iSL = 0; iSL < 5; ++iSL) {
      string name = "TSF" + to_string(iSL * 2) + "_input_i";
      tsf[iSL] = Xsi_Instance.get_port_number_or_exit(name);
      tsfInput[iSL].fill(zero_val);
      Xsi_Instance.put_value(tsf[iSL], tsfInput[iSL].data());
    }
    out  = Xsi_Instance.get_port_number_or_exit("Main_out");

    // Start low clock
    Xsi_Instance.put_value(clk, &zero_val);
    Xsi_Instance.run(10);
  } catch (std::exception& e) {
    std::cerr << "ERROR: An exception occurred: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "ERROR: An unknown exception occurred." << std::endl;
    // Xsi_Instance.get_error_info();
    throw;
  }
}

int CDCTrigger2DFinderFirmware::localSegmentID(int globalID)
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

auto CDCTrigger2DFinderFirmware::encodeTSHit(CDCTriggerSegmentHit const* hit)
{
  bitset<8> id(localSegmentID(hit->getSegmentID()));
  bitset<9> priorityTime(hit->priorityTime());
  bitset<2> LR(hit->getLeftRight());
  bitset<2> priorityPosition(hit->getPriorityPosition());
  string joined = id.to_string() + priorityTime.to_string() +
                  LR.to_string() + priorityPosition.to_string();
  bitset<m_tsVectorWidth> tsInfo(joined);
  B2DEBUG(10, "encoded ts hit: " << tsInfo << " with local ID " << localSegmentID(hit->getSegmentID()));
  return tsInfo;
}

auto CDCTrigger2DFinderFirmware::toTSSLV(const std::bitset<m_tsVectorWidth> ts)
{
  std::string word = ts.to_string();
  std::array<char, m_tsVectorWidth> vec;
  for (unsigned i = 0; i < word.size(); ++i) {
    vec[i] = (word[i] == '0') ? zero_val : one_val;
  }
  return vec;
}

tsOut CDCTrigger2DFinderFirmware::decodeTSHit(string tsIn)
{
  constexpr unsigned lenID = 8;
  constexpr unsigned lenPriorityTime = 9;
  constexpr unsigned lenLR = 2;
  constexpr unsigned lenPriorityPosition = 2;
  constexpr array<unsigned, 4> tsLens = {lenID, lenPriorityTime, lenLR, lenPriorityPosition};
  array<unsigned, 5> tsPos = { 0 };
  partial_sum(tsLens.begin(), tsLens.end(), tsPos.begin() + 1);
  tsOut tsOutput;
  tsOutput[0] = bitset<tsLens[0]>(tsIn.substr(tsPos[0], tsLens[0])).to_ulong();
  tsOutput[1] = bitset<tsLens[1]>(tsIn.substr(tsPos[1], tsLens[1])).to_ulong();
  tsOutput[2] = bitset<tsLens[2]>(tsIn.substr(tsPos[2], tsLens[2])).to_ulong();
  tsOutput[3] = bitset<tsLens[3]>(tsIn.substr(tsPos[3], tsLens[3])).to_ulong();
  return tsOutput;
}

TRGFinderTrack CDCTrigger2DFinderFirmware::decodeTrack(string trackIn)
{
  constexpr unsigned lenCharge = 2;
  constexpr unsigned lenOmega = 7;
  constexpr unsigned lenPhi0 = 7;
  constexpr unsigned lenTS = 21;
  constexpr array<unsigned, 3> trackLens = {lenCharge, lenOmega, lenPhi0};
  array<unsigned, 4> trackPos{ 0 };
  partial_sum(trackLens.begin(), trackLens.end(), trackPos.begin() + 1);
  const unsigned shift = 16 - lenOmega;
  TRGFinderTrack trackOut;
  bitset<trackLens[1]> omega(trackIn.substr(trackPos[1], trackLens[1]));
  // shift omega to 16 bits, cast it to signed 16-bit int, and shift it back to 7 bits
  // thus the signed bit is preserved (when right-shifting)
  int omegaFirm = (int16_t (omega.to_ulong() << shift)) >> shift;
  trackOut.omega = 29.97 * 1.5e-4 * omegaFirm;
  int phi0 = bitset<trackLens[2]>(trackIn.substr(trackPos[2], trackLens[2])).to_ulong();
  trackOut.phi0 = pi() / 4 + 90. / 80 * (phi0 + 1);
  for (unsigned i = 0; i < 5; ++i) {
    trackOut.ts[i] = decodeTSHit(trackIn.substr(trackPos.back() + i * lenTS, lenTS));
  }
  return trackOut;
}

void CDCTrigger2DFinderFirmware::decodeOutput(short latency)
{
  const unsigned lenTrack = 121;
  array<int, 4> posTrack;
  for (unsigned i = 0; i < posTrack.size(); ++i) {
    posTrack[i] = 6 + lenTrack * i;
  }
  string strOutput = slv_to_bin_string(finderOutput);
  StoreArray<CDCTriggerTrack> storeTracks(m_mod->m_outputCollectionName);
  for (unsigned i = 0; i < m_nOutTracksPerClock; ++i) {
    if (finderOutput[i] == one_val) {
      TRGFinderTrack trk = decodeTrack(strOutput.substr(posTrack[i], lenTrack));
      const CDCTriggerTrack* track =
        storeTracks.appendNew(trk.phi0, trk.omega, 0., latency);
      // TODO: dig out the TS hits in datastore, and
      // add relations to them
      B2DEBUG(10, "phi0:" << trk.phi0 << ", omega:" << trk.omega
              << ", at clock " << latency);
    }
  }
}

void CDCTrigger2DFinderFirmware::event()
{
  int status = 0;

  StoreArray<CDCTriggerSegmentHit> tsHits(m_mod->m_hitCollectionName);

  B2DEBUG(10, tsHits.getEntries() << " hits in this event");
  // index of TS hits, sorted by TS found time
  // std::vector<int> iHit(tsHits.getEntries());
  // std::iota(iHit.begin(), iHit.end(), 0);
  std::vector<int> iHit;
  for (int i = 0; i < tsHits.getEntries(); ++i) {
    if (tsHits[i]->getISuperLayer() % 2 == 0) {
      iHit.push_back(i);
    }
  }
  std::sort(iHit.begin(), iHit.end(), [tsHits](int i, int j) {
    return tsHits[i]->foundTime() < tsHits[j]->foundTime();
  });

  for (auto hit : iHit) {
    B2DEBUG(10, "sorted found time: " << tsHits[hit]->foundTime());
  }
  // data clock period (32ns) in unit of 2ns
  const int clockPeriod = 16;

  // classify hits by clocks
  // number of clocks before the last input hit
  int nClocks = tsHits[iHit.back()]->foundTime() / clockPeriod;
  // each element of seqHit holds a list of all the hits within a clock cycle
  vector<vector<int> > seqHit(nClocks);
  auto itrHit = iHit.begin();
  // For each clock,
  for (int i = 0; i < nClocks; ++i) {
    if (tsHits[*itrHit]->foundTime() < 0) {
      B2WARNING("Negative found time! The clock assignment will be wrong.");
    }
    // Fill in all the hits whose found time is less than the next clock edge,
    while (tsHits[*itrHit]->foundTime() < (i + 1) * clockPeriod &&
           itrHit != iHit.end()) {
      seqHit[i].push_back(*itrHit);
      ++itrHit;
    }
    if (seqHit[i].size() > m_nInTSPerClock) {
      B2WARNING("Oops! Too many input hits in a clock!");
    }
  }

  const int nAxialSuperLayer = 5;
  // this is "typedef" using C++11 syntax
  using tsArrayPerClock = vector<bitset<m_tsVectorWidth> >;
  using tsArrayPerSL = vector<tsArrayPerClock>;
  using tsArray = array<tsArrayPerSL, nAxialSuperLayer>;

  tsArray tsHitVector;
  for (auto& sl : tsHitVector) {
    sl.resize(nClocks);
  }

  // assign hits to SLV
  for (unsigned iClock = 0; iClock < seqHit.size(); ++iClock) {
    for (auto iTS : seqHit[iClock]) {
      B2DEBUG(10, "encoding ts ID: " << tsHits[iTS]->getSegmentID() <<
              ", SL " << tsHits[iTS]->getISuperLayer() <<
              ", clock " << iClock
             );
      bitset<m_tsVectorWidth> ts(encodeTSHit(tsHits[iTS]));
      tsHitVector[tsHits[iTS]->getISuperLayer() / 2][iClock].push_back(ts);
    }
  }

  // assign input signals
  try {
    for (int iClock = 0; iClock < m_nClockPerEvent; iClock++) {
      // Put clk to one
      Xsi_Instance.put_value(clk, &one_val);
      Xsi_Instance.run(10);

      std::cout << "------------------" << "\n";
      std::cout << "clock #" << iClock << "\n";
      for (unsigned iSL = 0; iSL < nAxialSuperLayer; ++iSL) {
        // firstly, clear the input signal vectors
        tsfInput[iSL].fill(zero_val);
        // then, copy the TS info to the input signal vectors
        // unless there is no more hit in the SL
        auto itr = tsfInput[iSL].begin() + 9;
        if (iClock < nClocks) {
          for (auto ts : tsHitVector[iSL][iClock]) {
            tsVector inTS(toTSSLV(ts));
            copy(inTS.begin(), inTS.end(), itr);
            advance(itr, m_tsVectorWidth);
          }
        }
        // cout << display_value(tsfInput[iSL].data(), 9) << " " <<
        //      display_value(tsfInput[iSL].data() + 9, 210) << "\n";
        cout << "TSF" << iSL << " input: ";
        display_hex(tsfInput[iSL]);
        // write the input
        Xsi_Instance.put_value(tsf[iSL], tsfInput[iSL].data());
      }

      // read the output
      Xsi_Instance.get_value(out, t2d_out);

      // Put clk to zero
      Xsi_Instance.put_value(clk, &zero_val);
      Xsi_Instance.run(10);

      string found = display_value(t2d_out, 6);
      std::cout << "output: " << found << "\n";
      for (unsigned j = 0; j < count(found.begin(), found.end(), '1'); j++) {
        std::cout << display_value(t2d_out + 6 + 121 * j, 121) << "\n";
      }
      finderOutput = to_array(t2d_out);
      decodeOutput(iClock);
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
