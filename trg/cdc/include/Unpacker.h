#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <numeric>
#include <cmath>
#include <algorithm>

#include <trg/cdc/dataobjects/Bitstream.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerFinderClone.h>
#include <framework/gearbox/Const.h>

namespace Belle2 {
  namespace CDCTriggerUnpacker {

    constexpr double pi() { return std::atan(1) * 4; }

    // constants
    static constexpr std::array<int, 9> nMergers = {10, 10, 12, 14, 16, 18, 20, 22, 24};

    // The data width are the specs under full speed.
    // When using one of the half-speed version,
    // TS/track are filled from MSB, leaving LSB blank.
    static constexpr int TSFOutputWidth = 429;
    static constexpr int nTrackers = 4;
    static constexpr int nAxialTSF = 5;
    static constexpr int T2DOutputWidth = 747;

    static constexpr int nMax2DTracksPerClock = 6;

    // bit width of the clock counter
    static constexpr int clockCounterWidth = 9;

    /* number of mergers in axial super layers */
    static constexpr std::array<int, nAxialTSF> nAxialMergers = {10, 12, 16, 20, 24};
    /* number of wires in a super layer*/
    static constexpr std::array<int, 9> nWiresInSuperLayer = {
      160, 160, 192, 224, 256, 288, 320, 352, 384
    };
    /* Number of wire/cells in a single layer per merger unit */
    static constexpr int nCellsInLayer = 16;

    // dataobjects
    using TSFOutputVector = std::array<char, TSFOutputWidth>;
    using TSFOutputArray = std::array<TSFOutputVector, nTrackers>;
    using TSFOutputBus = std::array<TSFOutputArray, nAxialTSF>;
    using TSFOutputBitStream = Bitstream<TSFOutputBus>;

    using T2DOutputVector = std::array<char, T2DOutputWidth>;
    using T2DOutputBus = std::array<T2DOutputVector, nTrackers>;
    using T2DOutputBitStream = Bitstream<T2DOutputBus>;

    // functions
    void printBuffer(int* buf, int nwords)
    {
      for (int j = 0; j < nwords; ++j) {
        printf(" %.8x", buf[j]);
        if ((j + 1) % 8 == 0) {
          printf("\n");
        }
      }
      printf("\n");
      return;
    };

    std::string rawIntToAscii(int buf)
    {
      std::ostringstream firmwareTypeStream;
      firmwareTypeStream << std::hex << buf;
      std::string firmwareTypeHex(firmwareTypeStream.str());
      std::string firmwareType(4, '0');
      for (int i = 0; i < 4; i++) {
        std::istringstream firmwareTypeIStream(firmwareTypeHex.substr(i * 2, 2));
        int character;
        firmwareTypeIStream >> std::hex >> character;
        firmwareType[i] = character;
      }
      return firmwareType;
    };

    std::string rawIntToString(int buf)
    {
      std::ostringstream firmwareVersionStream;
      firmwareVersionStream << std::hex << buf;
      return firmwareVersionStream.str();
    };

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

    char std_logic(bool inBit)
    {
      char outBit = zero_val;
      if (inBit) {
        outBit = one_val;
      }
      return outBit;
    }

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
      int ini = padding ? 4 - signal.size() % 4 : 0;
      std::string res(ini, '0');
      for (auto const& bit : signal) {
        if (bit >= 0 && bit < 9) {
          res += std_logic_literal[(int) bit];
        } else {
          B2WARNING("invalid signal detected: " << static_cast<int>(bit));
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


    /**
     *  Calculate the global TS ID from the ID in a super layer
     *
     *  @param localID     Segment ID in TSF output
     *
     *  @param iSL         Super layer ID (0-8)
     */
    unsigned short globalSegmentID(unsigned short localID, unsigned short iSL)
    {
      auto itr = nWiresInSuperLayer.begin();
      unsigned short globalID = std::accumulate(itr, itr + iSL, 0);
      globalID += localID;
      return globalID;
    }

    using tsOut = std::array<unsigned, 4>;
    using tsOutArray = std::array<tsOut, 5>;
    struct TRG2DFinderTrack {
      double omega;
      double phi0;
      tsOutArray ts;
    };

    /**
     *  Calculate the local TS ID (continuous ID in a super layer)
     *
     *  @param tsIDInTracker    Segment ID in a single tracker
     *
     *  @param iAx              Axial super layer ID (0-4)
     *
     *  @param iTracker         ID of the tracker
     *
     *  @return the local TS ID
     */
    unsigned TSIDInSL(unsigned tsIDInTracker, unsigned iAx, unsigned iTracker)
    {
      const unsigned nCellsInSL = nAxialMergers[iAx] * nCellsInLayer;
      // get global TS ID
      unsigned iTS = tsIDInTracker + nCellsInSL * iTracker / nTrackers;
      // periodic ID overflow when phi0 > 0 for the 4th tracker
      if (iTS >= nCellsInSL) {
        iTS -= nCellsInSL;
      }
      // ID in SL8 is shifted by 16
      if (iAx == 4) {
        if (iTS < 16) {
          iTS += nCellsInSL;
        }
        iTS -= 16;
      }
      return iTS;
    }

    /**
     *  Decode the track segment hit from the bit string
     *
     *  @param tsIn    21-bit string of the TS information
     *
     *  @return        tsOut (ID, priority time, L/R, priority position)
     */
    tsOut decodeTSHit(std::string tsIn)
    {
      constexpr unsigned lenID = 8;
      constexpr unsigned lenPriorityTime = 9;
      constexpr unsigned lenLR = 2;
      constexpr unsigned lenPriorityPosition = 2;
      constexpr std::array<unsigned, 4> tsLens = {
        lenID, lenPriorityTime, lenLR, lenPriorityPosition
      };
      std::array<unsigned, 5> tsPos = { 0 };
      std::partial_sum(tsLens.begin(), tsLens.end(), tsPos.begin() + 1);
      tsOut tsOutput;
      tsOutput[0] = std::bitset<tsLens[0]>(tsIn.substr(tsPos[0], tsLens[0])).to_ulong();
      tsOutput[1] = std::bitset<tsLens[1]>(tsIn.substr(tsPos[1], tsLens[1])).to_ulong();
      tsOutput[2] = std::bitset<tsLens[2]>(tsIn.substr(tsPos[2], tsLens[2])).to_ulong();
      tsOutput[3] = std::bitset<tsLens[3]>(tsIn.substr(tsPos[3], tsLens[3])).to_ulong();
      return tsOutput;
    }

    /**
     *  Decode the 2D finder track from the bit string
     *
     *  @param trackIn  121-bit string of the TS information
     *
     *  @return         TRG2DFinderTrack containing omega, phi0 and related TS hit
     */
    TRG2DFinderTrack decode2DTrack(std::string trackIn)
    {
      constexpr unsigned lenCharge = 2;
      constexpr unsigned lenOmega = 7;
      constexpr unsigned lenPhi0 = 7;
      constexpr unsigned lenTS = 21;
      constexpr std::array<unsigned, 3> trackLens = {lenCharge, lenOmega, lenPhi0};
      std::array<unsigned, 4> trackPos{ 0 };
      std::partial_sum(trackLens.begin(), trackLens.end(), trackPos.begin() + 1);
      const unsigned shift = 16 - lenOmega;
      TRG2DFinderTrack trackOut;
      std::bitset<trackLens[1]> omega(trackIn.substr(trackPos[1], trackLens[1]));
      // shift omega to 16 bits, cast it to signed 16-bit int, and shift it back to 7 bits
      // thus the signed bit is preserved (when right-shifting)
      int omegaFirm = (int16_t (omega.to_ulong() << shift)) >> shift;
      // B field is 1.5T
      const double BField = 1.5e-4;
      // omega in 1/cm
      // omega = 1/R = c * B / pt
      // c.f. https://confluence.desy.de/download/attachments/34033650/output-def.pdf
      trackOut.omega = Const::speedOfLight * BField / 0.3 / 34 * omegaFirm;
      int phi0 = std::bitset<trackLens[2]>(trackIn.substr(trackPos[2], trackLens[2])).to_ulong();
      trackOut.phi0 = pi() / 4 + pi() / 2 / 80 * (phi0 + 1);
      for (unsigned i = 0; i < 5; ++i) {
        trackOut.ts[i] = decodeTSHit(trackIn.substr(trackPos.back() + i * lenTS, lenTS));
      }
      return trackOut;
    }

    /**
     *  Decode the 2D finder output from the Bitstream
     *
     *  @param foundTime    the clock at which this track appears
     *
     *  @param bits         pointer to the output Bitstream
     *
     *  @param storeTracks  pointer to the track StoreArray
     *
     *  @param storeClones  pointer to the StoreArray holding clone info
     *
     *  @param tsHits       pointer to the TS hit StoreArray
     *
     */
    void decode2DOutput(short foundTime,
                        T2DOutputBitStream* bits,
                        StoreArray<CDCTriggerTrack>* storeTracks,
                        StoreArray<CDCTriggerFinderClone>* storeClones,
                        StoreArray<CDCTriggerSegmentHit>* tsHits)
    {
      const unsigned lenTrack = 121;
      const unsigned oldTrackWidth = 6;
      const unsigned foundWidth = 6;
      std::array<int, 4> posTrack;
      for (unsigned i = 0; i < posTrack.size(); ++i) {
        posTrack[i] = oldTrackWidth + foundWidth + lenTrack * i;
      }
      for (unsigned iTracker = 0; iTracker < nTrackers; ++iTracker) {
        const auto slv = bits->signal()[iTracker];
        std::string strOutput = slv_to_bin_string(slv).
                                substr(clockCounterWidth, T2DOutputWidth - clockCounterWidth);
        for (unsigned i = 0; i < nMax2DTracksPerClock; ++i) {
          // The first 6 bits indicate whether a track is found or not
          if (slv[clockCounterWidth + oldTrackWidth + i] == one_val) {
            TRG2DFinderTrack trk = decode2DTrack(strOutput.substr(posTrack[i], lenTrack));
            // rotate the tracks from 2D1 to 2D3
            double globalPhi0 = trk.phi0 + pi() / 2 * iTracker;
            if (globalPhi0 > pi() * 2) {
              globalPhi0 -= pi() * 2;
            }
            B2DEBUG(15, "phi0:" << globalPhi0 << ", omega:" << trk.omega
                    << ", at clock " << foundTime << ", tracker " << iTracker);
            CDCTriggerTrack* track =
              storeTracks->appendNew(globalPhi0, trk.omega, 0., foundTime);
            CDCTriggerFinderClone* clone =
              storeClones->appendNew(slv[clockCounterWidth + i] == one_val, iTracker);
            clone->addRelationTo(track);
            // TODO: dig out the TS hits in DataStore, and
            // add relations to them.
            // Otherwise, create a new TS hit object and add the relation.
            // However, the fastest time would be lost in this case.
            // Problem: there might be multiple TS hits with the same ID,
            // so the foundTime needs to be aligned first in order to compare.
            for (unsigned iAx = 0; iAx < nAxialTSF; ++iAx) {
              const auto& ts = trk.ts[iAx];
              if (ts[3] > 0) {
                unsigned iTS = TSIDInSL(ts[0], iAx, iTracker);
                CDCTriggerSegmentHit* hit =
                  tsHits->appendNew(2 * iAx, // super layer
                                    iTS, // TS number in super layer
                                    ts[3], // priority position
                                    ts[2], // L/R
                                    ts[1], // priority time
                                    0, // fastest time (unknown)
                                    0); // found time (unknown)
                track->addRelationTo(hit);
              }
            }
          }
        }
      }
    }

    /**
     *  Decode the 2D finder input from the Bitstream
     *
     *  @param foundTime    the clock at which a TS hit appears
     *
     *  @param timeOffset   offset of the foundTime for each 2D
     *
     *  @param bits         pointer to the input Bitstream
     *
     *  @param tsHits       pointer to the TS hit StoreArray
     *
     */
    void decode2DInput(short foundTime,
                       std::array<int, 4> timeOffset,
                       TSFOutputBitStream* bits,
                       StoreArray<CDCTriggerSegmentHit>* tsHits)
    {
      constexpr unsigned lenTS = 21;
      // Get the input TS to 2D from the Bitstream
      for (unsigned iAx = 0; iAx < nAxialTSF; ++iAx) {
        for (unsigned iTracker = 0; iTracker < nTrackers; ++iTracker) {
          const auto& tracker = bits->signal()[iAx][iTracker];
          std::string strInput = slv_to_bin_string(tracker);
          bool noMoreHit = false;
          for (unsigned pos = clockCounterWidth; pos < TSFOutputWidth; pos += lenTS) {
            std::string tsHitStr = strInput.substr(pos, lenTS);
            B2DEBUG(50, tsHitStr);
            tsOut ts = decodeTSHit(tsHitStr);
            // check if all the hits are on the MSB side
            if (ts[2] == 0) {
              noMoreHit = true;
              continue;
            } else if (noMoreHit) {
              B2WARNING("Discontinuous TS hit detected!");
            }
            unsigned iTS = TSIDInSL(ts[0], iAx, iTracker);
            // Make TS hit object
            CDCTriggerSegmentHit hit(2 * iAx, // super layer
                                     iTS, // TS number in super layer
                                     ts[3], // priority position
                                     ts[2], // L/R
                                     ts[1], // priority time
                                     0, // fastest time (unknown)
                                     foundTime + timeOffset[iTracker]); // found time

            // add if the TS hit of identical ID and foundTime is not already in the StoreArray
            // (from the 2D input of another quarter or the 2D track output)

            /* TODO: Currently, it is very likely that a TS hit will appear
             * multiple times in the StoreArray. To avoid adding the input from
             * another quarter again, we need to look at the clock counter,
             * because the data from different 2D's are not always synchronized
             * due to Belle2Link instability. To avoid adding again from the 2D
             * output, we need to consider the 2D latency.
             */
            if (std::none_of(tsHits->begin(), tsHits->end(),
            [hit](CDCTriggerSegmentHit storeHit) {
            return (storeHit.getSegmentID() == hit.getSegmentID() &&
                    storeHit.foundTime() == hit.foundTime());
            })) {
              B2DEBUG(40, "found TS hit ID " << hit.getSegmentID() <<
                      ", SL" << 2 * iAx << ", local ID " << iTS <<
                      ", 2D" << iTracker);
              tsHits->appendNew(hit);
            } else {
              B2DEBUG(45, "skipping redundant hit ID " << hit.getSegmentID() << " in 2D" << iTracker);
            }
          }
        }
      }
    }
  }
}
