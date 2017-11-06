/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tzu-An Sheng                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <trg/cdc/modules/trgcdc/CDCTriggerTSFFirmwareModule.h>
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

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

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
    for (int i = 0; i < size; i++) {
      if (count[i] >= 0 && count[i] < 9) {
        res += std_logic_literal[(int) count[i]];
      } else {
        B2WARNING("invalid signal detected: " << static_cast<int>(count[i]));
        res += "?";
      }
    }
    return res;
  }

  template<size_t N>
  std::string slv_to_bin_string(std::array<char, N> signal, bool padding = false)
  {
    int ini = padding ? signal.size() % 4 : 0;
    std::string res(ini, '0');
    for (auto const& bit : signal) {
      if (bit >= 0 && bit < 9) {
        res += std_logic_literal[(int) bit];
      } else {
        B2WARNING("nonvalid signal detected: " << static_cast<int>(bit));
        res += "0";
      }
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
      B2WARNING("Some bit in the signal vector is neither 0 nor 1. \n" <<
                "Displaying binary values instead.");
      std::cout << slv_to_bin_string(signal) << std::endl;
    } else {
      std::string binString = slv_to_bin_string(signal, true);
      std::cout << std::setfill('0');
      for (unsigned i = 0; i < signal.size(); i += 4) {
        std::bitset<4> set(binString.substr(i, 4));
        std::cout << std::setw(1) << std::hex << set.to_ulong();
      }
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
    // loader.emplace_back(new Xsi::Loader(design_libname_pre + to_string(i * 2)
    //                                     + design_libname_post, simengine_libname));
    inputToTSF[i].resize(nMergers[i]);
    array<char, 256> mer;
    mer.fill(3);
    fill(inputToTSF[i].begin(), inputToTSF[i].end(), mer);
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
  // for (auto& l : loader) {
  //   l->close();
  // }
}

void
writer(const char* message, int count, FILE* stream, FILE* invstream)
{
  char buffer[1024];
  for (; count > 0 ; --count) {
    fprintf(stream, "%s\n" , message);
    fflush(stream);

    // inverse
    fgets(buffer, sizeof(buffer), invstream);
    fputs(buffer, stdout);
  }
}

#include <ext/stdio_filebuf.h>

using __gnu_cxx::stdio_filebuf;
using std::istream;
using std::ostream;

inline stdio_filebuf<char>* fileBufFromFD(int fd, std::_Ios_Openmode mode)
{
  return (new stdio_filebuf<char> (fd, mode));
}

istream* createInStreamFromFD(int fd)
{
  stdio_filebuf<char>* fileBuf = fileBufFromFD(fd, std::ios::in);
  return (new istream(fileBuf));
}

ostream* createOutStreamFromFD(int fd)
{
  stdio_filebuf<char>* fileBuf = fileBufFromFD(fd, std::ios::out);
  return (new ostream(fileBuf));
}

CDCTriggerTSFFirmwareModule::outputArray
CDCTriggerTSFFirmwareModule::writeRead(const char* message, FILE* outstream, FILE* instream)
{
  // write input to TSF firmware
  fprintf(outstream, "%s\n" , message);
  fflush(outstream);
  if (getLogConfig().getDebugLevel() >= 50) {
    usleep(200000);
  }
  // read output from TSF firmware
  array<char, 2048> buffer;
  outputArray output;
  buffer.fill(3);
  if (fgets(buffer.data(), buffer.size(), instream) == NULL) {
    B2ERROR("fgets reached end unexpectedly");
    return output;
  }
  // ins->getline(buffer.data(), buffer.size());
  B2DEBUG(50, "display received TSF output:");
  if (getLogConfig().getDebugLevel() >= 50) {
    for (auto i2d = 0; i2d < 4; ++i2d) {
      B2DEBUG(50, display_value(buffer.data() + width_out * i2d, width_out));
    }
  }
  auto bufferItr = buffer.cbegin();
  for (int iTracker = 0; iTracker < nTrackers; ++iTracker) {
    copy(bufferItr, bufferItr + width_out, output[iTracker].begin());
    bufferItr += width_out;
  }
  return output;
}

void CDCTriggerTSFFirmwareModule::initialize()
{
  // m_cdcHits.isRequired(m_hitCollectionName);

  std::cout << "Design DLL     : " << design_libname_pre << "*" <<
            design_libname_post << std::endl;
  std::cout << "Sim Engine DLL : " << simengine_libname << std::endl;

  // try {
  for (unsigned i = 0; i < m_nSubModules; ++i) {
    if (i == 1) continue;
    // if (i >= 3) continue;
    // i: input to worker (output from module)
    // o: output from worker (input to module)
    /* Create pipe and place the two-end pipe file descriptors*/
    pipe(inputFileDescriptor[i].data());
    pipe(outputFileDescriptor[i].data());
    string str_fd[] = {to_string(inputFileDescriptor[i][0]), to_string(outputFileDescriptor[i][1])};
    pid_t pid = fork();
    if (pid < 0) {
      B2FATAL("Fork failed!");
    } else if (pid == (pid_t) 0) {
      /* Child process (consumer) */
      close(inputFileDescriptor[i][1]);
      close(outputFileDescriptor[i][0]);
      string design = design_libname_pre + to_string(i * 2) + design_libname_post;
      string waveform = wdbName_pre + to_string(i * 2) + wdbName_post;
      execlp("CDCTriggerTSFFirmwareWorker", "CDCTriggerTSFFirmwareWorker",
             str_fd[0].c_str(), str_fd[1].c_str(), design.c_str(), waveform.c_str(),
             to_string(nMergers[i]).c_str(), nullptr);
      B2FATAL("The firmware simulation program didn't launch!");
    } else {
      /* Parent process (producer)  */
      B2DEBUG(10, "parent " << i);
      m_pid[i] = pid;
      // Close the copy of the fds read end
      close(inputFileDescriptor[i][0]);
      close(outputFileDescriptor[i][1]);
      // open the fds
      stream[i][0] = fdopen(inputFileDescriptor[i][1], "w");
      stream[i][1] = fdopen(outputFileDescriptor[i][0], "r");
      // ins = createInStreamFromFD(outputFileDescriptor[i][0]);
      B2DEBUG(20, "pid for TSF" << i * 2 << ": " << m_pid[i]);
    }
  }
  // } catch (std::exception& e) {
  //   std::cerr << "ERROR: An exception occurred: " << e.what() << std::endl;
  // } catch (...) {
  //   std::cerr << "ERROR: An unknown exception occurred." << std::endl;
  //   // Xsi_Instance.get_error_info();
  //   throw;
  // }
}

void CDCTriggerTSFFirmwareModule::terminate()
{
  B2DEBUG(10, "Waiting for TSF firmware termination...");
  wait();
  for (unsigned i = 0; i < m_nSubModules; ++i) {
    if (i == 1) continue;
    // if (i >= 3) continue;
    close(inputFileDescriptor[i][1]);
    close(outputFileDescriptor[i][0]);
  }
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
    return m_cdcHits[i]->getTDCCount() > m_cdcHits[j]->getTDCCount();
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
        // skip problematic TSF2 simulation
        if (iSL == 1) continue;
        // if (iSL >= 3) continue;
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
        // if (getLogConfig().getDebugLevel())
        B2DEBUG(100, "input to TSF" << iSL * 2 << ": \n" <<
                display_value(rawInputToTSF[iSL], nMergers[iSL] * width_in));
        cout << flush;
        // write the TSF input
        // fprintf(stream[iSL][0], "%s\n", rawInputToTSF[iSL]);

        outputToTracker[iSL] = writeRead(rawInputToTSF[iSL], stream[iSL][0], stream[iSL][1]);
        // for (const auto& out : outputToTracker[iSL]) {
        //   display_hex(out);
        // }

        // read the TSF output
        // cout << "reading output from TSF firmware:" << endl;
        // fgets(outputToTracker[iSL].data(), width_in * nMergers[iSL], stream[iSL][1]);

      }
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
