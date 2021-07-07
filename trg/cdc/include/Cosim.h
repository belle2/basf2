/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/logging/Logger.h>

#include <string>
#include <iostream>
#include <iomanip>
#include <bitset>

#include <unistd.h>

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

  /** In case you are not familiar with VHDL simulation, there are 9 possible
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

  /**Display value of the signal*/
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

  /**Transform into string*/
  template<size_t N>
  std::string slv_to_bin_string(std::array<char, N> signal, bool padding = false)
  {
    int ini = padding ? signal.size() % 4 : 0;
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

  /**Display signal in hex*/
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

  /**Display nonzero value of signal in hex*/
  template<size_t N>
  void display_nonzero_hex(const std::array<char, N>& signal)
  {
    if (std::any_of(signal.begin(), signal.end(), [](char i)
    {return i != zero_val;})) {
      display_hex(signal);
    }
  }

  /** extract a subset of bitstring, like substring.
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

std::string getcurrentdir()
{
  char buf[1024];
  getcwd(buf, sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = 0;
  return buf;
}
