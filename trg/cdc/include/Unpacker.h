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
#include <trg/cdc/dataobjects/CDCTriggerMLPInput.h>
#include <framework/gearbox/Const.h>

namespace Belle2 {
  namespace CDCTriggerUnpacker {

    constexpr double pi() { return std::atan(1) * 4; }

    // constants
    static constexpr std::array<int, 9> nMergers = {10, 10, 12, 14, 16, 18, 20, 22, 24};

    // The data width are the specs under full speed.
    // When using one of the half-speed version,
    // TS/track are filled from MSB, leaving LSB blank.
    static constexpr int TSFOutputWidth = TSF_TO_2D_WIDTH; // 429 (#defined in Bitstream.h)
    static constexpr int nTrackers = NUM_2D; // 4
    static constexpr int nAxialTSF = NUM_TSF; // 5
    static constexpr int nStereoTSF = 4;
    static constexpr int T2DOutputWidth = T2D_TO_3D_WIDTH; // 747
    static constexpr int NNInputWidth = NN_IN_WIDTH; // 982
    static constexpr int NNOutputWidth = NN_OUT_WIDTH; // 570
    static constexpr unsigned lenTS = 21;   // ID (8 bit) + t (9 bit) + LR (2 bit) + priority (2 bit)

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

    using NNInputVector = std::array<char, NNInputWidth>;
    using NNInputBus = std::array<NNInputVector, nTrackers>;
    using NNInputBitStream = Bitstream<NNInputBus>;

    using NNOutputVector = std::array<char, NNOutputWidth>;
    using NNOutputBus = std::array<NNOutputVector, nTrackers>;
    using NNOutputBitStream = Bitstream<NNOutputBus>;

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
    struct TRGNeuroTrack {
      double z;
      double theta;
      unsigned sector;
      std::array<float, 9> inputID;
      std::array<float, 9> inputT;
      std::array<float, 9> inputAlpha;
      std::array<tsOut, 9> ts;
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
    unsigned TSIDInSL(unsigned tsIDInTracker, unsigned iSL, unsigned iTracker)
    {
      const unsigned nCellsInSL = nMergers[iSL] * nCellsInLayer;
      // get global TS ID
      unsigned iTS = tsIDInTracker + nCellsInSL * iTracker / nTrackers;
      // periodic ID overflow when phi0 > 0 for the 4th tracker
      if (iTS >= nCellsInSL) {
        iTS -= nCellsInSL;
      }
      // ID in SL8 is shifted by 16
      if (iSL == 8) {
        if (iTS < 16) {
          iTS += nCellsInSL;
        }
        iTS -= 16;
      }
      return iTS;
    }

    /** Convert 13-bit string to signed int
     *  (typical encoding of MLP input and output) */
    int mlp_bin_to_signed_int(std::string signal)
    {
      constexpr unsigned len = 13;
      std::bitset<len> signal_bit(signal);
      const unsigned shift = 16 - len;
      // shift to 16 bits, cast it to signed 16-bit int, and shift it back
      // thus the signed bit is preserved (when right-shifting)
      int signal_out = (int16_t (signal_bit.to_ulong() << shift)) >> shift;
      return signal_out;
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
     *  @param iTracker ID of the tracker
     *
     *  @return         TRG2DFinderTrack containing omega, phi0 and related TS hit
     */
    TRG2DFinderTrack decode2DTrack(std::string trackIn, unsigned iTracker)
    {
      constexpr unsigned lenCharge = 2;
      constexpr unsigned lenOmega = 7;
      constexpr unsigned lenPhi0 = 7;
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

      // rotate the tracks to the correct quadrant (iTracker)
      double globalPhi0 = trackOut.phi0 + pi() / 2 * iTracker;
      if (globalPhi0 > pi() * 2) {
        globalPhi0 -= pi() * 2;
      }
      trackOut.phi0 = globalPhi0;
      return trackOut;
    }

    /** Decode the neuro track from the bit string
     *
     *  @param trackIn  NNOutput string from the clock cycle containing the neural network results
     *
     *  @param selectIn NNOutput string from the clock cycle containing the input selection (TS and input vector)
     *
     *  @return         TRGNeuroTrack containing z, theta, sector, MLP input and related TS hit
     */
    TRGNeuroTrack decodeNNTrack(std::string trackIn, std::string selectIn)
    {
      constexpr unsigned lenMLP = 13;
      float scale = 1. / (1 << (lenMLP - 1));
      TRGNeuroTrack foundTrack;
      int theta_raw = mlp_bin_to_signed_int(trackIn.substr(1, lenMLP));
      foundTrack.theta = theta_raw * scale * M_PI_2 + M_PI_2;
      int z_raw = mlp_bin_to_signed_int(trackIn.substr(lenMLP + 1, lenMLP));
      foundTrack.z = z_raw * scale * 50.;
      foundTrack.sector = std::bitset<3>(trackIn.substr(2 * lenMLP + 1, 3)).to_ulong();
      for (unsigned iSL = 0; iSL < 9; ++iSL) {
        foundTrack.inputAlpha[iSL] =
          mlp_bin_to_signed_int(selectIn.substr((2 + (8 - iSL)) * lenMLP + 4, lenMLP)) * scale;
        foundTrack.inputT[iSL] =
          mlp_bin_to_signed_int(selectIn.substr((11 + (8 - iSL)) * lenMLP + 4, lenMLP)) * scale;
        foundTrack.inputID[iSL] =
          mlp_bin_to_signed_int(selectIn.substr((20 + (8 - iSL)) * lenMLP + 4, lenMLP)) * scale;
        foundTrack.ts[iSL] =  // order: SL8, ..., SL0
          decodeTSHit(selectIn.substr(29 * lenMLP + 4 + (8 - iSL) * lenTS, lenTS));
      }
      return foundTrack;
    }

    /** Search for hit in datastore and add it, if there is no matching hit.
     *
     *  @return   pointer to the hit (existing or newly created)
     */
    CDCTriggerSegmentHit* addTSHit(tsOut ts, unsigned iSL, unsigned iTracker,
                                   StoreArray<CDCTriggerSegmentHit>* tsHits,
                                   int foundTime = 0)
    {
      unsigned iTS = TSIDInSL(ts[0], iSL, iTracker);
      // check if hit is already existing in datastore
      CDCTriggerSegmentHit* hit = nullptr;
      //for (int ihit = 0; ihit < tsHits->getEntries(); ++ihit) {
      //  CDCTriggerSegmentHit* compare = (*tsHits)[ihit];
      //  if (compare->getISuperLayer() == iSL &&
      //      compare->getIWireCenter() == iTS &&
      //      compare->getPriorityPosition() == ts[3] &&
      //      compare->getLeftRight() == ts[2] &&
      //      compare->priorityTime() == int(ts[1])) {
      //    hit = compare;
      //    break;
      //  }
      //}
      if (!hit) {
        hit = tsHits->appendNew(iSL, iTS, ts[3], ts[2], ts[1], 0, foundTime, iTracker);
        B2DEBUG(15, "make hit at SL " << iSL << " ID " << iTS << " clock " << foundTime << " iTracker " << iTracker);
      }
      return hit;
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
            TRG2DFinderTrack trk = decode2DTrack(strOutput.substr(posTrack[i], lenTrack), iTracker);
            B2DEBUG(15, "2DOut phi0:" << trk.phi0 << ", omega:" << trk.omega
                    << ", at clock " << foundTime << ", tracker " << iTracker);
            CDCTriggerTrack* track =
              storeTracks->appendNew(trk.phi0, trk.omega, 0., foundTime, iTracker);
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
                unsigned iTS = TSIDInSL(ts[0], 2 * iAx, iTracker);
                CDCTriggerSegmentHit* hit =
                  tsHits->appendNew(2 * iAx, // super layer
                                    iTS, // TS number in super layer
                                    ts[3], // priority position
                                    ts[2], // L/R
                                    ts[1], // priority time
                                    0, // fastest time (unknown)
                                    foundTime, // found time (using the unpacked clock cycle)
                                    iTracker); // quadrant
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
            unsigned iTS = TSIDInSL(ts[0], 2 * iAx, iTracker);
            // Make TS hit object
            CDCTriggerSegmentHit hit(2 * iAx, // super layer
                                     iTS, // TS number in super layer
                                     ts[3], // priority position
                                     ts[2], // L/R
                                     ts[1], // priority time
                                     0, // fastest time (unknown)
                                     foundTime + timeOffset[iTracker], // found time
                                     iTracker);  // quadrant

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

    /**
     *  Decode the neurotrigger input from the Bitstream
     *
     *  @param foundTime      the clock at which the input appears
     *
     *  @param bitsIn         pointer to the input Bitstream
     *
     *  @param store2DTracks  pointer to the 2D track StoreArray
     *
     *  @param tsHits         pointer to the TS hit StoreArray
     *
     *  @param iTracker       the current tracker id
     *
     *  @param trk2D          adress where a found 2D track is stored
     *
     *  @return bool          is there a 2D track?
     *
     */
    bool decodeNNInput(short iclock,
                       unsigned iTracker,
                       NNInputBitStream* bitsIn,
                       StoreArray<CDCTriggerTrack>* store2DTracks,
                       StoreArray<CDCTriggerSegmentHit>* tsHits,
                       TRG2DFinderTrack* trk2D)
    {
      constexpr unsigned lenTrack = 119;  // omega (7 bit) + phi (7 bit) + 5 * TS (21 bit)
      const auto slvIn = bitsIn->signal()[iTracker];
      std::string strIn = slv_to_bin_string(slvIn);
      // decode stereo hits
      for (unsigned iSt = 0; iSt < nStereoTSF; ++iSt) {
        for (unsigned iHit = 0; iHit < 10; ++iHit) {
          // order: 10 * SL7, 10 * SL5, 10 * SL3, 10 * SL1
          unsigned pos = ((nStereoTSF - iSt - 1) * 10 + iHit) * lenTS;
          tsOut ts = decodeTSHit(strIn.substr(pos, lenTS));
          if (ts[3] > 0) {
            addTSHit(ts, iSt * 2 + 1, iTracker, tsHits, iclock);
          }
        }
      }
      std::string strTrack = strIn.substr(nStereoTSF * 10 * lenTS, lenTrack);
      if (!std::all_of(strTrack.begin(), strTrack.end(), [](char i) {return i == '0';})) {
        strTrack = "00" + strTrack.substr(5 * lenTS, 14) + strTrack.substr(0,
                   5 * lenTS); // add 2 dummy bits for the charge (not stored in NN)
        *trk2D = decode2DTrack(strTrack, iTracker);
        B2DEBUG(15, "NNIn phi0:" << trk2D->phi0 << ", omega:" << trk2D->omega
                << ", at clock " << iclock << ", tracker " << iTracker);
        // check if 2D track is already in list, otherwise add it
        CDCTriggerTrack* track2D = nullptr;
        //for (int itrack = 0; itrack < store2DTracks->getEntries(); ++itrack) {
        //  if ((*store2DTracks)[itrack]->getPhi0() == trk2D->phi0 &&
        //      (*store2DTracks)[itrack]->getOmega() == trk2D->omega) {
        //    track2D = (*store2DTracks)[itrack];
        //    B2DEBUG(15, "found 2D track in store with phi " << trk2D->phi0 << " omega " << trk2D->omega);
        //    break;
        //  }
        //}
        if (!track2D) {
          B2DEBUG(15, "make new 2D track with phi " << trk2D->phi0 << " omega " << trk2D->omega << " clock " << iclock);
          track2D = store2DTracks->appendNew(trk2D->phi0, trk2D->omega, 0., iclock, iTracker);
        }
        // add axial hits if not present already and create relations
        for (unsigned iAx = 0; iAx < nAxialTSF; ++iAx) {
          const auto& ts = trk2D->ts[iAx];
          if (ts[3] > 0) {
            CDCTriggerSegmentHit* hit =
              addTSHit(ts, 2 * iAx, iTracker, tsHits, iclock);
            track2D->addRelationTo(hit);
          }
        }
        // TODO: decode event time
        return true; // 2D track found
      }
      return false;  // 2D track not found
    }

    /**
     *  Decode the neurotrigger output from the Bitstream
     *
     *  @param foundTime      the clock at which the output appears
     *
     *  @param iTracker       the current tracker id
     *
     *  @param bitsOut        pointer to the output Bitstream containing the neural network results
     *
     *  @param bitsSelectTS   pointer to the output Bitstream containing the related TS
     *
     *  @param storeNNTracks  pointer to the NN track StoreArray
     *
     *  @param store2DTracks  pointer to the 2D track StoreArray
     *
     *  @param tsHits         pointer to the TS hit StoreArray
     *
     *  @param storeNNInputs  pointer to the NN Input StoreArray
     *
     *  @param trk2D          pointer to the related 2D track
     *
     */
    void decodeNNOutput(short foundTime,
                        unsigned iTracker,
                        NNOutputBitStream* bitsOut,
                        NNOutputBitStream* bitsSelectTS,
                        StoreArray<CDCTriggerTrack>* storeNNTracks,
                        StoreArray<CDCTriggerTrack>* store2DTracks,
                        StoreArray<CDCTriggerSegmentHit>* tsHits,
                        StoreArray<CDCTriggerMLPInput>* storeNNInputs,
                        TRG2DFinderTrack* trk2D)
    {
      const auto slvOut = bitsOut->signal()[iTracker];
      std::string strTrack = slv_to_bin_string(slvOut);
      const auto slvSelect = bitsSelectTS->signal()[iTracker];
      std::string strSelect = slv_to_bin_string(slvSelect);
      TRGNeuroTrack trkNN = decodeNNTrack(strTrack, strSelect);
      B2DEBUG(15, "make new NN track with , z:" << trkNN.z << ", theta:" << trkNN.theta <<
              ", sector:" << trkNN.sector << ", clock " << foundTime);
      CDCTriggerTrack* trackNN = storeNNTracks->appendNew(trk2D->phi0, trk2D->omega, 0.,
                                                          trkNN.z, cos(trkNN.theta) / sin(trkNN.theta), 0., foundTime, iTracker);
      std::vector<float> inputVector(27, 0.);
      for (unsigned iSL = 0; iSL < 9; ++iSL) {
        inputVector[3 * iSL] = trkNN.inputID[iSL];
        inputVector[3 * iSL + 1] = trkNN.inputT[iSL];
        inputVector[3 * iSL + 2] = trkNN.inputAlpha[iSL];
      }
      CDCTriggerMLPInput* storeInput =
        storeNNInputs->appendNew(inputVector, trkNN.sector);
      trackNN->addRelationTo(storeInput);
      trackNN->addRelationTo((*store2DTracks)[store2DTracks->getEntries() - 1]);

      for (unsigned iSL = 0; iSL < 9; ++iSL) {
        if (trkNN.ts[iSL][3] > 0) {
          CDCTriggerSegmentHit* hit = addTSHit(trkNN.ts[iSL] , iSL, iTracker, tsHits, foundTime);
          trackNN->addRelationTo(hit);
        }
      }
    }

    /**
     *  Decode the neurotrigger input and output from the Bitstream
     *
     *  @param bitsToNN       pointer to the Input Bitstream StoreArray (combination of stereo TS and single 2D track)
     *
     *  @param bitsFromNN     pointer to the Output Bitstream StoreArray (including preprocessing results)
     *
     *  @param store2DTracks  pointer to the 2D track StoreArray
     *
     *  @param storeNNTracks  pointer to the NN track StoreArray
     *
     *  @param tsHits         pointer to the TS hit StoreArray
     *
     *  @param storeNNInputs  pointer to the NN Input StoreArray
     *
     */
    void decodeNNIO(
      StoreArray<CDCTriggerUnpacker::NNInputBitStream>* bitsToNN,
      StoreArray<CDCTriggerUnpacker::NNOutputBitStream>* bitsFromNN,
      StoreArray<CDCTriggerTrack>* store2DTracks,
      StoreArray<CDCTriggerTrack>* storeNNTracks,
      StoreArray<CDCTriggerSegmentHit>* tsHits,
      StoreArray<CDCTriggerMLPInput>* storeNNInputs)
    {
      constexpr short delayNNOutput = 8;
      constexpr short delayNNSelect = 3;
      for (short iclock = 0; iclock < bitsFromNN->getEntries(); ++iclock) {
        NNInputBitStream* bitsIn = (*bitsToNN)[iclock];
        NNOutputBitStream* bitsOutEnable = (*bitsFromNN)[iclock];
        for (unsigned iTracker = 0; iTracker < nTrackers; ++iTracker) {
          const auto slvOutEnable = bitsOutEnable->signal()[iTracker];
          std::string stringOutEnable = slv_to_bin_string(slvOutEnable);
          if (stringOutEnable.c_str()[0] == '1') {
            TRG2DFinderTrack trk2D;
            bool has2D = decodeNNInput(iclock, iTracker, bitsIn, store2DTracks, tsHits, &trk2D);
            if (has2D) {
              short foundTime = iclock + delayNNOutput;
              if (foundTime  < bitsFromNN->getEntries()) {
                NNOutputBitStream* bitsOut = (*bitsFromNN)[foundTime];
                NNOutputBitStream* bitsSelectTS = (*bitsFromNN)[iclock + delayNNSelect];
                decodeNNOutput(iclock, iTracker, bitsOut, bitsSelectTS,
                               storeNNTracks, store2DTracks, tsHits, storeNNInputs,
                               &trk2D);
              }
            }
          }
        }
      }
    }
  }
}
