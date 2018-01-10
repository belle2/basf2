#include <string>
#include <sstream>

namespace CDCTriggerUnpacker {

  // constants
  static constexpr std::array<int, 9> nMergers = {10, 10, 12, 14, 16, 18, 20, 22, 24};


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
