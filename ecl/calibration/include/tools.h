/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>

namespace Belle2 {
  namespace ECL {

    /// function that creates 2D vector
    template<typename T>
    inline std::vector<std::vector<T>> create2Dvector(unsigned int n1, unsigned int n2)
    {
      std::vector<std::vector<T>>  vec(n1);
      for (auto& v2 : vec) v2.resize(n2);
      return vec;
    }


    /// function that creates 3D vector
    template<typename T>
    inline std::vector<std::vector<std::vector<T>>> create3Dvector(unsigned int n1, unsigned int n2, unsigned int n3)
    {
      std::vector<std::vector<std::vector<T>>>  vec(n1);
      for (auto& v2 : vec) {
        v2.resize(n2);
        for (auto& v3 : v2)
          v3.resize(n3);
      }
      return vec;
    }


    /// function that creates 4D vector
    template<typename T>
    inline std::vector<std::vector<std::vector<std::vector<T>>>> create4Dvector(unsigned int n1, unsigned int n2, unsigned int n3,
        unsigned int n4)
    {
      std::vector<std::vector<std::vector<std::vector<T>>>>  vec(n1);
      for (auto& v2 : vec) {
        v2.resize(n2);
        for (auto& v3 : v2) {
          v3.resize(n3);
          for (auto& v4 : v3)
            v4.resize(n4);
        }
      }
      return vec;
    }


  }
} // namespace Belle2
