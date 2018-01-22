#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <trg/cdc/dataobjects/Bitstream.h>

namespace Belle2 {
  namespace CDCTriggerUnpacker {

    // constants
    static constexpr std::array<int, 9> nMergers = {10, 10, 12, 14, 16, 18, 20, 22, 24};

    // The data width are the specs under full speed.
    // When using one of the half-speed version,
    // TS/track are filled from MSB, leaving LSB blank.
    static constexpr int TSFOutputWidth = 429;
    static constexpr int nTrackers = 4;
    static constexpr int nAxialTSF = 5;
    static constexpr int T2DOutputWidth = 741;

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
  }
}
