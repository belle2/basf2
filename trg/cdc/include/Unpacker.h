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
#include <trg/cdc/dbobjects/CDCTriggerNeuroConfig.h>

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
    static constexpr unsigned lenTS = 21;   // ID (8 bit) + t (9 bit) + LR (2 bit) + priority (2 bit)

    static constexpr int nMax2DTracksPerClock = 4;

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

    using NNVector = std::array<char, NN_WIDTH>;
    using NNBus = std::array<NNVector, nTrackers>;
    using NNBitStream = Bitstream<NNBus>;
    // using NNInputVector = std::array<char, NNInputWidth>;
    // using NNInputBus = std::array<NNInputVector, nTrackers>;
    // using NNInputBitStream = Bitstream<NNInputBus>;

    // using NNOutputVector = std::array<char, NNOutputWidth>;
    // using NNOutputBus = std::array<NNOutputVector, nTrackers>;
    // using NNOutputBitStream = Bitstream<NNOutputBus>;

    // functions
    std::string padto(std::string s, unsigned l)
    {
      if (s.size() < l) {
        s.insert(s.begin(), l - s.size(), ' ');
      }
      return s;
    }
    std::string padright(std::string s, unsigned l)
    {
      if (s.size() < l) {
        s.insert(s.end(), l - s.size(), ' ');
      }
      return s;
    }

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
          B2DEBUG(20, "invalid signal detected: " << static_cast<int>(count[i]));
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
          B2DEBUG(20, "invalid signal detected: " << static_cast<int>(bit));
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
        B2DEBUG(20, "Some bit in the signal vector is neither 0 nor 1. \n" <<
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

    static const double realNaN = std::numeric_limits<double>::quiet_NaN();

    using tsOut = std::array<unsigned, 4>;
    using tsOutArray = std::array<tsOut, 5>;
    /// TRG 2DFinder Track
    struct TRG2DFinderTrack {
      TRG2DFinderTrack() : omega(realNaN), phi0(realNaN) {}
      /// omega of a 2D track
      double omega;
      /// phi0 of a 2D track
      double phi0;
      /// all TS of a 2D track
      tsOutArray ts;
    };
    /// TRG Neuro track
    struct TRGNeuroTrack {
      /// z0 of a NN track
      double z;
      /// theta of a NN track
      double theta;
      /// sector of a NN track
      unsigned sector;
      /// input ID list of a NN track
      std::array<float, 9> inputID;
      /// input T list of a NN track
      std::array<float, 9> inputT;
      /// input Alpha list of a NN track
      std::array<float, 9> inputAlpha;
      /// input TS list of a NN track
      std::array<tsOut, 9> ts;
    };
    struct B2LDataField {
      B2LDataField(
        StoreArray<CDCTriggerUnpacker::NNBitStream>* bitsNN,
        unsigned foundtime,
        unsigned iTracker,
        const CDCTriggerNeuroConfig::B2FormatLine& b2line)
      {
        if (int(b2line.offset + foundtime) >= 0 &&
            int(b2line.offset + foundtime) <= bitsNN->getEntries()) {

          NNBitStream* bitsn = (*bitsNN)[foundtime + b2line.offset];

          if (int(slv_to_bin_string(bitsn->signal()[iTracker]).size()) >= (NN_WIDTH - b2line.start)) {
            data = slv_to_bin_string(bitsn->signal()[iTracker]).substr(NN_WIDTH - 1 - b2line.end, b2line.end - b2line.start + 1);
          } else {
            data = "";
          }
        } else {
          data = "";
        }

        // std::cout << "new datafield: " << b2line.name << ": " << b2line.start << ", " << b2line.end << ", " << data << std::endl;

        name = b2line.name;
      }
      std::string data;
      std::string name;
    };

    std::vector<bool> decodedriftthreshold(std::string p_driftthreshold)
    {
      std::vector<bool> res;
      for (unsigned i = 0; i < p_driftthreshold.size(); ++i) {
        if (p_driftthreshold.substr(i, 1) == "1") {
          res.push_back(true);
        } else if (p_driftthreshold.substr(i, 1) == "0") {
          res.push_back(false);
        } else {
          B2WARNING("Invalid input in NNBitstream appending 'false'!");
          res.push_back(false);
        }
      }
      return res;
    }
    std::vector<bool> decodefoundoldtrack(std::string p_foundoldtrack)
    {
      std::vector<bool> res;
      for (unsigned i = 0; i < p_foundoldtrack.size(); ++i) {
        if (p_foundoldtrack.substr(i, 1) == "1") {
          res.push_back(true);
        } else if (p_foundoldtrack.substr(i, 1) == "0") {
          res.push_back(false);
        } else {
          B2WARNING("Invalid input in NNBitstream appending 'false'!");
          res.push_back(false);
        }
      }
      return res;
    }
    bool decodevalstereobit(const std::string& p_valstereobit)
    {
      bool res;
      if (p_valstereobit == "1") {
        res = true;
      } else if (p_valstereobit == "0") {
        res = false;
      } else {
        B2WARNING("Invalid input in NNBitstream appending 'false'!");
        res = false;
      }
      return res;
    }
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
      constexpr unsigned lenPriorityTime = 9; //(twodcc.size() > 0) ? 13 : 9;
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
    tsOut decodeTSHit_sim(std::string tsIn, std::string twodcc)
    {
      constexpr unsigned lenID = 8;
      constexpr unsigned lenPriorityTime = 9; //(twodcc.size() > 0) ? 13 : 9;
      constexpr unsigned lenLR = 2;
      constexpr unsigned lenPriorityPosition = 2;
      constexpr std::array<unsigned, 4> tsLens = {
        lenID, lenPriorityTime, lenLR, lenPriorityPosition
      };
      std::string C = tsIn.substr(lenID + 5, 4);
      std::string B = tsIn.substr(lenID, 5);
      std::string Bp = twodcc.substr(4, 5);
      std::string Ap = twodcc.substr(0, 4);
      int pt;
      std::string pts;
      if (std::stoul(B, 0, 2) <= std::stoul(Bp, 0, 2)) {
        pts = Ap + B + C;
      } else {
        B2DEBUG(14, "2DCC overflow detected!");
        pts = std::bitset<4>(std::stoul(Ap, 0, 2) - 1).to_string() + B + C;
      }
      pt = std::stoul(pts, 0, 2);
      std::array<unsigned, 5> tsPos = { 0 };
      std::partial_sum(tsLens.begin(), tsLens.end(), tsPos.begin() + 1);
      tsOut tsOutput;
      tsOutput[0] = std::bitset<tsLens[0]>(tsIn.substr(tsPos[0], tsLens[0])).to_ulong();
      tsOutput[1] = pt; // std::bitset<tsLens[1]>(tsIn.substr(tsPos[1], tsLens[1])).to_ulong();
      tsOutput[2] = std::bitset<tsLens[2]>(tsIn.substr(tsPos[2], tsLens[2])).to_ulong();
      tsOutput[3] = std::bitset<tsLens[3]>(tsIn.substr(tsPos[3], tsLens[3])).to_ulong();
      return tsOutput;
    }
    tsOut decodeTSHit_ext(std::string tsIn, std::string expt)
    {
      constexpr unsigned lenID = 8;
      constexpr unsigned lenPriorityTime = 9; //(twodcc.size() > 0) ? 13 : 9;
      constexpr unsigned lenLR = 2;
      constexpr unsigned lenPriorityPosition = 2;
      constexpr std::array<unsigned, 4> tsLens = {
        lenID, lenPriorityTime, lenLR, lenPriorityPosition
      };
      unsigned pt = std::stoul(expt, 0, 2);
      std::array<unsigned, 5> tsPos = { 0 };
      std::partial_sum(tsLens.begin(), tsLens.end(), tsPos.begin() + 1);
      tsOut tsOutput;
      tsOutput[0] = std::bitset<tsLens[0]>(tsIn.substr(tsPos[0], tsLens[0])).to_ulong();
      tsOutput[1] = pt; //std::bitset<13>(expt).to_ulong();
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
    TRG2DFinderTrack decode2DTrack(const std::string& p_charge __attribute__((unused)),
                                   std::string p_omega,
                                   std::string p_phi,
                                   const std::string& p_ts0,
                                   const std::string& p_ts2,
                                   const std::string& p_ts4,
                                   const std::string& p_ts6,
                                   const std::string& p_ts8,
                                   unsigned iTracker,
                                   const std::string& p_2dcc,
                                   bool sim13dt)
    {
//constexpr unsigned lenomega = p_omega.size();
      unsigned shift = 16 - p_omega.size();
      TRG2DFinderTrack trackout;
      int omega = std::stoi(p_omega, 0, 2);
      // shift omega to 16 bits, cast it to signed 16-bit int, and shift it back to 7 bits
      // thus the signed bit is preserved (when right-shifting)
      int omegafirm = (int16_t (omega << shift)) >> shift;
      //int omegafirm = (int16_t (omega.to_ulong() << shift)) >> shift;
      // B field is 1.5T
      const double BField = 1.5e-4; // why is it so small?
      // omega in 1/cm
      // omega = 1/R = c * B / pt
      trackout.omega = Const::speedOfLight * BField / 0.3 / 34 * omegafirm;




      int phi = std::stoi(p_phi, 0, 2);
      // c.f. https://confluence.desy.de/download/attachments/34033650/output-def.pdf
      double globalPhi0 = pi() / 4 + pi() / 2 / 80 * (phi + 1) + pi() / 2 * iTracker; // see document above


      trackout.ts[0] = (sim13dt) ? decodeTSHit_sim(p_ts0, p_2dcc) : decodeTSHit(p_ts0);
      trackout.ts[1] = (sim13dt) ? decodeTSHit_sim(p_ts2, p_2dcc) : decodeTSHit(p_ts2);
      trackout.ts[2] = (sim13dt) ? decodeTSHit_sim(p_ts4, p_2dcc) : decodeTSHit(p_ts4);
      trackout.ts[3] = (sim13dt) ? decodeTSHit_sim(p_ts6, p_2dcc) : decodeTSHit(p_ts6);
      trackout.ts[4] = (sim13dt) ? decodeTSHit_sim(p_ts8, p_2dcc) : decodeTSHit(p_ts8);
      // rotate the tracks to the correct quadrant (iTracker)
      if (globalPhi0 > pi() * 2) {
        globalPhi0 -= pi() * 2;
      }
      trackout.phi0 = globalPhi0;
      B2DEBUG(20, "Unpacking 2DTrack in Tracker: " << iTracker);
      B2DEBUG(20, "    Omega: " << std::to_string(omega) << ", Omegafirm: " << std::to_string(omegafirm) << ", converted to: " <<
              std::to_string(trackout.omega));
      B2DEBUG(20, "    Phi:   " << std::to_string(phi) << ", converted to: " << std::to_string(trackout.phi0));
      return trackout;

    }
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
    TRGNeuroTrack decodeNNTrack(std::string p_mlpout_z,
                                std::string p_mlpout_theta,
                                std::string p_tsfsel,
                                std::string p_mlpin_alpha,
                                std::string p_mlpin_drifttime,
                                std::string p_mlpin_id,
                                std::string p_netsel,
                                const DBObjPtr<CDCTriggerNeuroConfig> neurodb,
                                const std::string& p_2dcc,
                                bool sim13dt,
                                B2LDataField p_extendedpts)
    {
      // constexpr unsigned lenMLP = 13;
      float scale_z = 1. / (1 << (p_mlpout_z.size() - 1));
      float scale_theta = 1. / (1 << (p_mlpout_theta.size() - 1));
      float scale_alpha = 1. / (1 << (p_mlpin_alpha.size() - 1) / 9);
      float scale_drifttime = 1. / (1 << (p_mlpin_drifttime.size() - 1) / 9);
      float scale_id = 1. / (1 << (p_mlpin_id.size() - 1) / 9);
      TRGNeuroTrack foundTrack;
      int theta_raw = mlp_bin_to_signed_int(p_mlpout_theta);
      int z_raw = mlp_bin_to_signed_int(p_mlpout_z);
      std::vector<float> unscaledT = neurodb->getMLPs()[0].unscaleTarget({(z_raw * scale_z), (theta_raw * scale_theta)});
      foundTrack.z = unscaledT[0];
      foundTrack.theta = unscaledT[1];
      foundTrack.sector = std::stoi(p_netsel, 0, 2);
      for (unsigned iSL = 0; iSL < 9; ++iSL) {
        foundTrack.inputAlpha[iSL] =
          mlp_bin_to_signed_int(p_mlpin_alpha.substr((8 - iSL) * p_mlpin_alpha.size() / 9, p_mlpin_alpha.size() / 9)) * scale_alpha;
        foundTrack.inputT[iSL] =
          mlp_bin_to_signed_int(p_mlpin_drifttime.substr((8 - iSL) * p_mlpin_drifttime.size() / 9,
                                                         p_mlpin_drifttime.size() / 9)) * scale_drifttime;
        foundTrack.inputID[iSL] =
          mlp_bin_to_signed_int(p_mlpin_id.substr((8 - iSL) * p_mlpin_drifttime.size() / 9, p_mlpin_drifttime.size() / 9)) * scale_id;
        if (sim13dt) {
          foundTrack.ts[iSL] = decodeTSHit_sim(p_tsfsel.substr((8 - iSL) * lenTS, lenTS), p_2dcc);
        } else {
          if (p_extendedpts.name != "None") {
            foundTrack.ts[iSL] = decodeTSHit_ext(p_tsfsel.substr((8 - iSL) * lenTS, lenTS), p_extendedpts.data.substr((8 - iSL) * 13, 13));
          } else {
            foundTrack.ts[iSL] = decodeTSHit(p_tsfsel.substr((8 - iSL) * lenTS, lenTS));
          }
        }
      }
      return foundTrack;
    }
    TRGNeuroTrack decodeNNTrack_old(std::string trackIn, std::string selectIn)
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
// !hit is always true.
//      if (!hit) {
      hit = tsHits->appendNew(iSL, iTS, ts[3], ts[2], ts[1], 0, foundTime, iTracker);
      B2DEBUG(15, "make hit at SL " << iSL << " ID " << iTS << " clock " << foundTime << " iTracker " << iTracker);
//      }
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
                                    // set to a clock definitely outside the time window to receive the TS from TSF,
                                    // so it won't cause any confusion of the TS origin.
                                    // what I want to is to set it to (200 + 2DmoduleID[0,1,2,3]),
                                    // so one can distinguish in which 2D this track is found.
                                    // foundTime); // found time (using the unpacked clock cycle)
                                    2000 + iTracker * 100 + foundTime,
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
              B2DEBUG(20, "Discontinuous TS hit detected!");
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
     *  @param foundTime          the clock at which the input appears
     *
     *  @param iTracker           the current tracker id
     *
     *  @param bitsIn             pointer to the input Bitstream
     *
     *  @param store2DTracks      pointer to the 2D track StoreArray
     *
     *  @param tsHits             pointer to the TS hit StoreArray
     *
     *  @return CDCTriggerTrack*  pointer to 2D track RelationsObject
     *
     */
    CDCTriggerTrack* decodeNNInput(short iclock,
                                   unsigned iTracker,
                                   NNBitStream* bitsIn,
                                   StoreArray<CDCTriggerTrack>* store2DTracks,
                                   StoreArray<CDCTriggerSegmentHit>* tsHits)
    {
      CDCTriggerTrack* track2D = nullptr;
      constexpr unsigned lenTrack = 135; //119;
      // omega (7 bit) + phi (7 bit) + 5 * TS (21 bit) + old track found(6bit) + valid stereo bit (1bit) + drift threshold (9bit)
      const auto slvIn = bitsIn->signal()[iTracker];
      std::string strIn = slv_to_bin_string(slvIn);
      strIn = strIn.substr(NN_WIDTH - 570 - 496, 982);
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
        std::string infobits = strTrack.substr(5 * lenTS + 14, 16);
        strTrack = "00" + strTrack.substr(5 * lenTS, 14) + strTrack.substr(0,
                   5 * lenTS); // add 2 dummy bits for the charge (not stored in NN)
        TRG2DFinderTrack trk2D = decode2DTrack(strTrack, iTracker);
        B2DEBUG(15, "NNIn phi0:" << trk2D.phi0 << ", omega:" << trk2D.omega
                << ", at clock " << iclock << ", tracker " << iTracker);
        B2DEBUG(300, "Content of new infobits: " << infobits);
        std::vector<bool> foundoldtrack;
        std::vector<bool> driftthreshold;
        bool valstereobit;
        unsigned i = 0;
        for (i = 0; i < 6; i++) {
          if (infobits.substr(i, 1) == "1") {
            foundoldtrack.push_back(true);
          } else if (infobits.substr(i, 1) == "0") {
            foundoldtrack.push_back(false);
          } else {
            B2WARNING("Invalid input in NNBitstream appending 'false'!");
            foundoldtrack.push_back(false);
          }
        }
        i = 6;
        if (infobits.substr(i, 1) == "1") {
          valstereobit = true;
        } else if (infobits.substr(i, 1) == "0") {
          valstereobit = false;
        } else {
          B2WARNING("Invalid input in NNBitstream appending 'false'!");
          valstereobit = false;
        }
        for (i = 7; i < 16; i++) {
          if (infobits.substr(i, 1) == "1") {
            driftthreshold.push_back(true);
          } else if (infobits.substr(i, 1) == "0") {
            driftthreshold.push_back(false);
          } else {
            B2WARNING("Invalid input in NNBitstream appending 'false'!");
            driftthreshold.push_back(false);
          }
        }
        B2DEBUG(15, "bits for foundoldtrack:    "   << foundoldtrack[0]
                << foundoldtrack[1]
                << foundoldtrack[2]
                << foundoldtrack[3]
                << foundoldtrack[4]
                << foundoldtrack[5]);
        B2DEBUG(15, "bits for driftthreshold:   "   << driftthreshold[0]
                << driftthreshold[1]
                << driftthreshold[2]
                << driftthreshold[3]
                << driftthreshold[4]
                << driftthreshold[5]
                << driftthreshold[6]
                << driftthreshold[7]
                << driftthreshold[8]);
        B2DEBUG(15, "bits for valstereobit:     "   << valstereobit);

        // check if 2D track is already in list, otherwise add it
        //for (int itrack = 0; itrack < store2DTracks->getEntries(); ++itrack) {
        //  if ((*store2DTracks)[itrack]->getPhi0() == trk2D->phi0 &&
        //      (*store2DTracks)[itrack]->getOmega() == trk2D->omega) {
        //    track2D = (*store2DTracks)[itrack];
        //    B2DEBUG(15, "found 2D track in store with phi " << trk2D->phi0 << " omega " << trk2D->omega);
        //    break;
        //  }
        //}
        B2DEBUG(15, "make new 2D track with phi " << trk2D.phi0 << " omega " << trk2D.omega << " clock " << iclock);
        track2D = store2DTracks->appendNew(trk2D.phi0, trk2D.omega, 0., foundoldtrack, driftthreshold, valstereobit, iclock, iTracker);
        // add axial hits and create relations
        for (unsigned iAx = 0; iAx < nAxialTSF; ++iAx) {
          const auto& ts = trk2D.ts[iAx];
          if (ts[3] > 0) {
            CDCTriggerSegmentHit* hit =
              addTSHit(ts, 2 * iAx, iTracker, tsHits, iclock);
            track2D->addRelationTo(hit);
          }
        }
        // TODO: decode event time
      }
      return track2D;
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
     *  @param tsHits         pointer to the TS hit StoreArray
     *
     *  @param storeNNInputs  pointer to the NN Input StoreArray
     *
     *  @param track2D        pointer to the related 2D track RelationsObject
     *
     */
    void decodeNNOutput_old(short foundTime,
                            unsigned iTracker,
                            NNBitStream* bitsOut,
                            NNBitStream* bitsSelectTS,
                            StoreArray<CDCTriggerTrack>* storeNNTracks,
                            StoreArray<CDCTriggerSegmentHit>* tsHits,
                            StoreArray<CDCTriggerMLPInput>* storeNNInputs,
                            CDCTriggerTrack* track2D)
    {
      const auto slvOut = bitsOut->signal()[iTracker];
      std::string strTrack = slv_to_bin_string(slvOut);
      strTrack = strTrack.substr(496, 570);
      const auto slvSelect = bitsSelectTS->signal()[iTracker];
      std::string strSelect = slv_to_bin_string(slvSelect);
      strSelect = strSelect.substr(496, 570);
      TRGNeuroTrack trkNN = decodeNNTrack_old(strTrack, strSelect);
      B2DEBUG(15, "make new NN track with , z:" << trkNN.z << ", theta:" << trkNN.theta <<
              ", sector:" << trkNN.sector << ", clock " << foundTime);
      double phi0 = 0;
      double omega = 0;
      if (track2D) {
        phi0 = track2D->getPhi0();
        omega = track2D->getOmega();
      }
      std::vector<bool> tsvector(9, false);
      for (unsigned iSL = 0; iSL < 9; ++iSL) {
        if (trkNN.ts[iSL][3] > 0) {
          tsvector[iSL] = true;
        }
      }
      CDCTriggerTrack* trackNN = storeNNTracks->appendNew(phi0, omega, 0.,
                                                          trkNN.z, cos(trkNN.theta) / sin(trkNN.theta), 0., track2D->getFoundOldTrack(), track2D->getDriftThreshold(),
                                                          track2D->getValidStereoBit(), trkNN.sector, tsvector, foundTime, iTracker);
      std::vector<float> inputVector(27, 0.);
      for (unsigned iSL = 0; iSL < 9; ++iSL) {
        inputVector[3 * iSL] = trkNN.inputID[iSL];
        inputVector[3 * iSL + 1] = trkNN.inputT[iSL];
        inputVector[3 * iSL + 2] = trkNN.inputAlpha[iSL];
      }
      CDCTriggerMLPInput* storeInput =
        storeNNInputs->appendNew(inputVector, trkNN.sector);
      trackNN->addRelationTo(storeInput);
      if (track2D) {
        track2D->addRelationTo(trackNN);
      }

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
      StoreArray<CDCTriggerUnpacker::NNBitStream>* bitsNN,
      StoreArray<CDCTriggerTrack>* store2DTracks,
      StoreArray<CDCTriggerTrack>* storeNNTracks,
      StoreArray<CDCTriggerSegmentHit>* tsHits,
      StoreArray<CDCTriggerSegmentHit>* tsHitsAll,
      StoreArray<CDCTriggerMLPInput>* storeNNInputs,
      const DBObjPtr<CDCTriggerNeuroConfig> neurodb,
      bool sim13dt)
    {
      for (unsigned iTracker = 0; iTracker < nTrackers; ++iTracker) {
        B2DEBUG(21, "----------------------------------------------------------------------------------------------------");
        B2DEBUG(21, padright("  Unpacking Tracker: " + std::to_string(iTracker), 100));
        // loop over boards belonging to geometrical sectors

        for (short iclock = 0; iclock < bitsNN->getEntries(); ++iclock) {
          // check for NNEnable bit:
          B2LDataField p_nnenable(bitsNN, iclock, iTracker, neurodb->getB2FormatLine("NNEnable"));
          if (p_nnenable.name == "None") {
            B2DEBUG(5, "Neurotrigger: NNENable position unknown, skipping ... ");
            continue;
          } else if (p_nnenable.data == "1") {
            B2DEBUG(10, padright("Tracker: " + std::to_string(iTracker) + ", Clock: " + std::to_string(iclock) + " : NNEnable set!", 100));
          } else {
            B2DEBUG(21, padright("    UnpackerClock: " + std::to_string(iclock), 100));
          }


          CDCTriggerNeuroConfig::B2FormatLine nnall;
          nnall.start = 0;
          nnall.end = 2047;
          nnall.offset = 0;
          nnall.name = "nnall";
          B2LDataField p_nnall(bitsNN, iclock, iTracker, nnall);
          B2DEBUG(22, padright("      all bits: ", 100));
          B2DEBUG(22, padright("        " + p_nnall.data, 100));
          // define variables to fill from the bitstream, B2LDataField holds just the string, not the unpacked data yet
          B2LDataField p_driftthreshold(bitsNN, iclock, iTracker, neurodb->getB2FormatLine("DriftThreshold"));
          if ((p_driftthreshold.name != "None") && (p_driftthreshold.data.size() == 0)) {
            B2DEBUG(10, "Could not load Datafield: " << p_driftthreshold.name << " from bitstream. Maybe offset was out of bounds? clock: " <<
                    iclock);
            continue;
          }

          B2LDataField p_valstereobit(bitsNN, iclock, iTracker, neurodb->getB2FormatLine("ValStereoBit"));
          if ((p_valstereobit.name != "None") && (p_valstereobit.data.size() == 0)) {
            B2DEBUG(10, "Could not load Datafield: " << p_valstereobit.name << " from bitstream. Maybe offset was out of bounds? clock: " <<
                    iclock);
            continue;
          }

          B2LDataField p_foundoldtrack(bitsNN, iclock, iTracker, neurodb->getB2FormatLine("FoundOldTrack"));
          if ((p_foundoldtrack.name != "None") && (p_foundoldtrack.data.size() == 0)) {
            B2DEBUG(10, "Could not load Datafield: " << p_foundoldtrack.name << " from bitstream. Maybe offset was out of bounds? clock: " <<
                    iclock);
            continue;
          }

          B2LDataField p_phi(bitsNN, iclock, iTracker, neurodb->getB2FormatLine("Phi"));
          if ((p_phi.name != "None") && (p_phi.data.size() == 0)) {
            B2DEBUG(10, "Could not load Datafield: " << p_phi.name << " from bitstream. Maybe offset was out of bounds? clock: " << iclock);
            continue;
          }

          B2LDataField p_omega(bitsNN, iclock, iTracker, neurodb->getB2FormatLine("Omega"));
          if ((p_omega.name != "None") && (p_omega.data.size() == 0)) {
            B2DEBUG(10, "Could not load Datafield: " << p_omega.name << " from bitstream. Maybe offset was out of bounds? clock: " << iclock);
            continue;
          }

          B2LDataField p_ts8(bitsNN, iclock, iTracker, neurodb->getB2FormatLine("TS8"));
          if ((p_ts8.name != "None") && (p_ts8.data.size() == 0)) {
            B2DEBUG(10, "Could not load Datafield: " << p_ts8.name << " from bitstream. Maybe offset was out of bounds? clock: " << iclock);
            continue;
          }

          B2LDataField p_ts6(bitsNN, iclock, iTracker, neurodb->getB2FormatLine("TS6"));
          if ((p_ts6.name != "None") && (p_ts6.data.size() == 0)) {
            B2DEBUG(10, "Could not load Datafield: " << p_ts6.name << " from bitstream. Maybe offset was out of bounds? clock: " << iclock);
            continue;
          }

          B2LDataField p_ts4(bitsNN, iclock, iTracker, neurodb->getB2FormatLine("TS4"));
          if ((p_ts4.name != "None") && (p_ts4.data.size() == 0)) {
            B2DEBUG(10, "Could not load Datafield: " << p_ts4.name << " from bitstream. Maybe offset was out of bounds? clock: " << iclock);
            continue;
          }

          B2LDataField p_ts2(bitsNN, iclock, iTracker, neurodb->getB2FormatLine("TS2"));
          if ((p_ts2.name != "None") && (p_ts2.data.size() == 0)) {
            B2DEBUG(10, "Could not load Datafield: " << p_ts2.name << " from bitstream. Maybe offset was out of bounds? clock: " << iclock);
            continue;
          }

          B2LDataField p_ts0(bitsNN, iclock, iTracker, neurodb->getB2FormatLine("TS0"));
          if ((p_ts0.name != "None") && (p_ts0.data.size() == 0)) {
            B2DEBUG(10, "Could not load Datafield: " << p_ts0.name << " from bitstream. Maybe offset was out of bounds? clock: " << iclock);
            continue;
          }

          B2LDataField p_tsf1(bitsNN, iclock, iTracker, neurodb->getB2FormatLine("TSF1"));
          if ((p_tsf1.name != "None") && (p_tsf1.data.size() == 0)) {
            B2DEBUG(10, "Could not load Datafield: " << p_tsf1.name << " from bitstream. Maybe offset was out of bounds? clock: " << iclock);
            continue;
          }

          B2LDataField p_tsf3(bitsNN, iclock, iTracker, neurodb->getB2FormatLine("TSF3"));
          if ((p_tsf3.name != "None") && (p_tsf3.data.size() == 0)) {
            B2DEBUG(10, "Could not load Datafield: " << p_tsf3.name << " from bitstream. Maybe offset was out of bounds? clock: " << iclock);
            continue;
          }

          B2LDataField p_tsf5(bitsNN, iclock, iTracker, neurodb->getB2FormatLine("TSF5"));
          if ((p_tsf5.name != "None") && (p_tsf5.data.size() == 0)) {
            B2DEBUG(10, "Could not load Datafield: " << p_tsf5.name << " from bitstream. Maybe offset was out of bounds? clock: " << iclock);
            continue;
          }

          B2LDataField p_tsf7(bitsNN, iclock, iTracker, neurodb->getB2FormatLine("TSF7"));
          if ((p_tsf7.name != "None") && (p_tsf7.data.size() == 0)) {
            B2DEBUG(10, "Could not load Datafield: " << p_tsf7.name << " from bitstream. Maybe offset was out of bounds? clock: " << iclock);
            continue;
          }

          B2LDataField p_tsfsel(bitsNN, iclock, iTracker, neurodb->getB2FormatLine("TSFsel"));
          if ((p_tsfsel.name != "None") && (p_tsfsel.data.size() == 0)) {
            B2DEBUG(10, "Could not load Datafield: " << p_tsfsel.name << " from bitstream. Maybe offset was out of bounds? clock: " << iclock);
            continue;
          }

          B2LDataField p_mlpin_alpha(bitsNN, iclock, iTracker, neurodb->getB2FormatLine("MLPIn_alpha"));
          if ((p_mlpin_alpha.name != "None") && (p_mlpin_alpha.data.size() == 0)) {
            B2DEBUG(10, "Could not load Datafield: " << p_mlpin_alpha.name << " from bitstream. Maybe offset was out of bounds? clock: " <<
                    iclock);
            continue;
          }

          B2LDataField p_mlpin_drifttime(bitsNN, iclock, iTracker, neurodb->getB2FormatLine("MLPIn_driftt"));
          if ((p_mlpin_drifttime.name != "None") && (p_mlpin_drifttime.data.size() == 0)) {
            B2DEBUG(10, "Could not load Datafield: " << p_mlpin_drifttime.name << " from bitstream. Maybe offset was out of bounds? clock: " <<
                    iclock);
            continue;
          }

          B2LDataField p_mlpin_id(bitsNN, iclock, iTracker, neurodb->getB2FormatLine("MLPIn_id"));
          if ((p_mlpin_id.name != "None") && (p_mlpin_id.data.size() == 0)) {
            B2DEBUG(10, "Could not load Datafield: " << p_mlpin_id.name << " from bitstream. Maybe offset was out of bounds? clock: " <<
                    iclock);
            continue;
          }

          B2LDataField p_netsel(bitsNN, iclock, iTracker, neurodb->getB2FormatLine("Netsel"));
          if ((p_netsel.name != "None") && (p_netsel.data.size() == 0)) {
            B2DEBUG(10, "Could not load Datafield: " << p_netsel.name << " from bitstream. Maybe offset was out of bounds? clock: " << iclock);
            continue;
          }

          B2LDataField p_mlpout_z(bitsNN, iclock, iTracker, neurodb->getB2FormatLine("MLPOut_z"));
          if ((p_mlpout_z.name != "None") && (p_mlpout_z.data.size() == 0)) {
            B2DEBUG(10, "Could not load Datafield: " << p_mlpout_z.name << " from bitstream. Maybe offset was out of bounds? clock: " <<
                    iclock);
            continue;
          }

          B2LDataField p_mlpout_theta(bitsNN, iclock, iTracker, neurodb->getB2FormatLine("MLPOut_theta"));
          if ((p_mlpout_theta.name != "None") && (p_mlpout_theta.data.size() == 0)) {
            B2DEBUG(10, "Could not load Datafield: " << p_mlpout_theta.name << " from bitstream. Maybe offset was out of bounds? clock: " <<
                    iclock);
            continue;
          }

          B2LDataField p_2dcc(bitsNN, iclock, iTracker, neurodb->getB2FormatLine("2dcc"));
          if ((p_2dcc.name != "None") && (p_2dcc.data.size() == 0)) {
            B2DEBUG(10, "Could not load Datafield: " << p_2dcc.name << " from bitstream. Maybe offset was out of bounds? clock: " << iclock);
            continue;
          }

          B2LDataField p_extendedpts(bitsNN, iclock, iTracker, neurodb->getB2FormatLine("extendedPriorityTimes"));
          if ((p_extendedpts.name != "None") && (p_extendedpts.data.size() == 0)) {
            B2DEBUG(10, "Could not load Datafield: " << p_extendedpts.name << " from bitstream. Maybe offset was out of bounds? clock: " <<
                    iclock);
            continue;
          }
          // B2LDataField  (bitsNN, iclock, iTracker, neurodb->getB2FormatLine(""));

          CDCTriggerTrack* track2D = nullptr;
          // decode stereo hits
          if (true) { // (p_nnenable.data == "1") {
            unsigned sln = 0;
            B2DEBUG(21, padright("      Stereos: ", 100));
            for (auto stereolayer : {p_tsf1, p_tsf3, p_tsf5, p_tsf7}) {
              if (stereolayer.name == "None") {
                B2ERROR("Error in CDCTriggerNeuroConfig Payload, position of stereo tsf could not be found!");
                continue;
              }
              std::string tsstr = " | ";
              for (unsigned iHit = 0; iHit < 10; ++iHit) {
                tsOut ts = (sim13dt) ? decodeTSHit_sim(stereolayer.data.substr(iHit * lenTS, lenTS),
                                                       p_2dcc.data) : decodeTSHit(stereolayer.data.substr(iHit * lenTS, lenTS));
                if (ts[3] > 0) { // if it is 0, it means 'no hit'
                  unsigned iTS = TSIDInSL(ts[0], sln * 2 + 1, iTracker);
                  tsstr += std::to_string(iTS) + ", " + std::to_string(ts[1]) + ", " + std::to_string(ts[2]) + ", " + std::to_string(ts[3]) + " | ";
                  addTSHit(ts, sln * 2 + 1, iTracker, tsHitsAll, iclock);
                }
              }
              B2DEBUG(21, padright("        SL" + std::to_string(sln * 2 + 1) + tsstr, 100));
              ++sln;
            }
          }
          B2DEBUG(21, padright("      2DCC: " + std::to_string(std::stoi(p_2dcc.data, 0, 2)) + ", (" + p_2dcc.data + ")", 100));
          if (p_nnenable.data == "1") {
            std::vector<bool> foundoldtrack{false};
            std::vector<bool> driftthreshold{false};
            bool valstereobit;
            if (p_foundoldtrack.name != "None") {
              foundoldtrack = decodefoundoldtrack(p_foundoldtrack.data);
            }
            if (p_driftthreshold.name != "None") {
              driftthreshold = decodedriftthreshold(p_driftthreshold.data);
            }
            if (p_valstereobit.name != "None") {
              valstereobit = decodevalstereobit(p_valstereobit.data);
            }

            if (std::all_of(p_phi.data.begin(), p_phi.data.end(), [](char i) {return i == 0;})) {
              B2ERROR("Empty Phi Value found for 2DTrack, should not happen!");
              continue;
            }
            TRG2DFinderTrack trk2D = decode2DTrack(
                                       "00", //charge
                                       p_omega.data,
                                       p_phi.data,
                                       p_ts0.data,
                                       p_ts2.data,
                                       p_ts4.data,
                                       p_ts6.data,
                                       p_ts8.data,
                                       iTracker,
                                       p_2dcc.data,
                                       sim13dt);
            track2D = store2DTracks->appendNew(trk2D.phi0, trk2D.omega, 0., foundoldtrack, driftthreshold, valstereobit, iclock, iTracker);
            B2DEBUG(12, padright("      2DTrack: (phi=" + std::to_string(trk2D.phi0) + ", omega=" + std::to_string(
                                   trk2D.omega) + ", update=" + std::to_string(foundoldtrack[1]) + ")", 100));

            // add axial hits and create relations
            std::string tsstr;
            for (unsigned iAx = 0; iAx < nAxialTSF; ++iAx) {
              const auto& ts = trk2D.ts[iAx];
              if (ts[3] > 0) {
                CDCTriggerSegmentHit* hit =
                  addTSHit(ts, 2 * iAx, iTracker, tsHitsAll, iclock);
                unsigned iTS = TSIDInSL(ts[0], iAx * 2, iTracker);
                tsstr += "(SL" + std::to_string(iAx * 2) + ", " + std::to_string(iTS) + ", " + std::to_string(ts[1]) + ", " + std::to_string(
                           ts[2]) + ", " + std::to_string(ts[3]) + "),";
                track2D->addRelationTo(hit);
              }
            }
            B2DEBUG(16, padright("      2DTrack TS: " + tsstr, 100));


            if (track2D) {
              TRGNeuroTrack trkNN;
              trkNN = decodeNNTrack(p_mlpout_z.data,
                                    p_mlpout_theta.data,
                                    p_tsfsel.data,
                                    p_mlpin_alpha.data,
                                    p_mlpin_drifttime.data,
                                    p_mlpin_id.data,
                                    p_netsel.data,
                                    neurodb,
                                    p_2dcc.data,
                                    sim13dt,
                                    p_extendedpts);


              B2DEBUG(11, padright("      NNTrack: (z=" + std::to_string(trkNN.z) + ", theta=" + std::to_string(trkNN.theta) + ")", 100));

              double phi0 = track2D->getPhi0();
              double omega = track2D->getOmega();

              std::vector<bool> tsvector(9, false);
              tsstr = "";
              // turns false, as soon as there is a ts, which is not contained in the 2dfindertrack
              bool isin2d = true;
              for (unsigned iSL = 0; iSL < 9; ++iSL) {
                if (trkNN.ts[iSL][3] > 0) {
                  tsvector[iSL] = true;
                  unsigned iTS = TSIDInSL(trkNN.ts[iSL][0], iSL, iTracker);
                  tsstr += "(SL" + std::to_string(iSL) + ", " + std::to_string(iTS) + ", " + std::to_string(trkNN.ts[iSL][1]) + ", " + std::to_string(
                             trkNN.ts[iSL][2]) + ", " + std::to_string(trkNN.ts[iSL][3]) + "),\n";
                  // check, wether axials are a subset of 2dfinderTS:
                  if (iSL % 2 == 0) {
                    if (!(trk2D.ts[iSL / 2][0] == trkNN.ts[iSL][0] &&
                          //trk2D.ts[iSL / 2][1] == trkNN.ts[iSL][1] &&
                          trk2D.ts[iSL / 2][2] == trkNN.ts[iSL][2] &&
                          trk2D.ts[iSL / 2][3] == trkNN.ts[iSL][3])) {
                      isin2d = false;
                    }
                  }

                } else {
                  tsstr += "( - ),\n";
                }
              }

              B2DEBUG(15, padright("      NNTrack TS: " + tsstr, 100));

              CDCTriggerTrack* trackNN = storeNNTracks->appendNew(phi0, omega, 0.,
                                                                  trkNN.z, cos(trkNN.theta) / sin(trkNN.theta), 0., track2D->getFoundOldTrack(), track2D->getDriftThreshold(),
                                                                  track2D->getValidStereoBit(), trkNN.sector, tsvector, iclock, iTracker);

              if (isin2d == false) {
                trackNN->setQualityVector(1);
              }
              std::vector<float> inputVector(27, 0.);
              for (unsigned iSL = 0; iSL < 9; ++iSL) {
                inputVector[3 * iSL] = trkNN.inputID[iSL];
                inputVector[3 * iSL + 1] = trkNN.inputT[iSL];
                inputVector[3 * iSL + 2] = trkNN.inputAlpha[iSL];
              }
              CDCTriggerMLPInput* storeInput =
                storeNNInputs->appendNew(inputVector, trkNN.sector);
              trackNN->addRelationTo(storeInput);
              track2D->addRelationTo(trackNN);

              for (unsigned iSL = 0; iSL < 9; ++iSL) {
                if (trkNN.ts[iSL][3] > 0) {
                  CDCTriggerSegmentHit* hit = nullptr;
                  // if (sim13dt) {
                  //   // get extended priority time for stereos from the allstereots storearray
                  //   for (int ihit = 0; ihit<tsHitsAll->getEntries(); ++ihit) {
                  //     CDCTriggerSegmentHit* compare = (*tsHitsAll)[ihit];
                  //     if (compare->getISuperLayer() == iSL &&
                  //         compare->getIWireCenter() == TSIDInSL(trkNN.ts[iSL][0], iSL, iTracker) &&
                  //         compare->getPriorityPosition() == trkNN.ts[iSL][3] &&
                  //         compare->getLeftRight() == trkNN.ts[iSL][2] ) {
                  //       hit = compare;
                  //     }
                  //   }
                  //   if (!(!hit)) {
                  //     tsHits->appendNew(hit->getISuperLayer(), hit->getIWireCenter(), hit->getPriorityPosition(), hit->getLeftRight(), hit->priorityTime(), 0, hit->foundTime(), iTracker);
                  //   }
                  //   // get the extended pt for axials from the already existing 2d TS
                  //   for (int ihit = 0; ihit<tsHits->getEntries(); ++ihit) {
                  //     CDCTriggerSegmentHit* compare = (*tsHits)[ihit];
                  //     if (compare->getISuperLayer() == iSL &&
                  //         compare->getIWireCenter() == TSIDInSL(trkNN.ts[iSL][0], iSL, iTracker) &&
                  //         compare->getPriorityPosition() == trkNN.ts[iSL][3] &&
                  //         compare->getLeftRight() == trkNN.ts[iSL][2] ) {
                  //       hit = compare;
                  //     }
                  //   }

                  // }

                  // cppcheck-suppress knownConditionTrueFalse
                  if (!hit) {
                    hit = addTSHit(trkNN.ts[iSL] , iSL, iTracker, tsHits, iclock);
                    //   B2DEBUG(1, "Hit with short drift time added, should not happen!");
                    // }
                  }
                  trackNN->addRelationTo(hit);
                  if (iSL % 2 == 0) {
                    track2D->addRelationTo(hit);
                  }
                }
              }
            }


            //
            //  B2DEBUG(15, "bits for foundoldtrack: ");
            //  for (auto x : foundoldtrack) {
            //    B2DEBUG(15, x);
            //  }
            //  B2DEBUG(15, "bits for driftthreshold:   ");
            //  for (auto x : driftthreshold) {
            //    B2DEBUG(15, x);
            //  }
            //  B2DEBUG(15, "bits for valstereobit:     "   << valstereobit);
            //  B2DEBUG(15, "make new 2D track with phi " << trk2D.phi0 << " omega " << trk2D.omega << " clock " << iclock);

          }
        }
      }
    }

    void decodeNNIO_old(
      StoreArray<CDCTriggerUnpacker::NNBitStream>* bitsNN,
      StoreArray<CDCTriggerTrack>* store2DTracks,
      StoreArray<CDCTriggerTrack>* storeNNTracks,
      StoreArray<CDCTriggerSegmentHit>* tsHits,
      StoreArray<CDCTriggerMLPInput>* storeNNInputs)
    {
      for (short iclock = 0; iclock < bitsNN->getEntries(); ++iclock) {
        NNBitStream* bitsIn = (*bitsNN)[iclock];
        NNBitStream* bitsOutEnable = (*bitsNN)[iclock];
        for (unsigned iTracker = 0; iTracker < nTrackers; ++iTracker) {
          const auto slvOutEnable = bitsOutEnable->signal()[iTracker];
          const auto slvIn = bitsIn->signal()[iTracker];
          std::string stringOutEnable = slv_to_bin_string(slvOutEnable); //.substr(NN_OUT_WIDTH - 570, NN_OUT_WIDTH);
          std::string stringIn = slv_to_bin_string(slvIn); //.substr(NN_OUT_WIDTH - 570, NN_OUT_WIDTH);
          if (stringOutEnable.c_str()[0] == '1') {
            CDCTriggerTrack* nntrack2D = decodeNNInput(iclock, iTracker, bitsIn, store2DTracks, tsHits);
            if (nntrack2D) {
              int foundTime = iclock;
              if (foundTime  < bitsNN->getEntries()) {
                NNBitStream* bitsOut = (*bitsNN)[foundTime];
                NNBitStream* bitsSelectTS = (*bitsNN)[iclock];
                decodeNNOutput_old(iclock, iTracker, bitsOut, bitsSelectTS,
                                   storeNNTracks, tsHits, storeNNInputs,
                                   nntrack2D);
              }
            }
          }
        }
      }
    }
  }
}
