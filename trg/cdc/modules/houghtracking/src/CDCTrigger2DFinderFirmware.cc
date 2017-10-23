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

#include <string>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <vector>
#include <array>
#include <algorithm>
#include <numeric>
#include <bitset>


namespace Cosim {
  const char* std_logic_literal[] = {"U", "X", "0", "1", "Z", "W", "L", "H", "-"};

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
    if (std::any_of(signal.begin(), signal.end(), [](char i) {return i != 2 && i != 3;})) {
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
      tsfInput[iSL].fill(2);
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
    vec[i] = (word[i] == '0') ? 2 : 3;
  }
  return vec;
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
        tsfInput[iSL].fill(2);
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
    }

    std::cout << status << std::endl;

  } catch (std::exception& e) {
    std::cerr << "ERROR: An exception occurred: " << e.what() << std::endl;
    status = 2;
  } catch (...) {
    std::cerr << "ERROR: An unknown exception occurred." << std::endl;
    status = 3;
  }

}

