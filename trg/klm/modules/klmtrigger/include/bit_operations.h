#pragma once

#include <bitset>
#include <stdint.h>
#include <algorithm>
#include <numeric>

namespace Belle2 {

  inline int countBits(uint64_t n)
  {
    return static_cast<int>(std::bitset<64>(n).count());
  }




  template <typename AXIS_NAME_T, typename CONTAINER_T>
  uint64_t to_bit_mask(const CONTAINER_T& container)
  {

    return std::accumulate(container.begin(), container.end(), uint64_t(0),
    [](const auto & lhs, const auto & rhs) {
      const auto bitshift = uint64_t(AXIS_NAME_T::get(rhs));
      if (bitshift > 32) {
        throw std::runtime_error("from:\nuint64_t to_bit_mask(const CONTAINER_T& container)\ninput number to large.\n\n");
      }
      return lhs | (uint64_t(1) << bitshift);
    });



  }

}