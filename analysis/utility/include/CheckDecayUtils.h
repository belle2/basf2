/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sourav Patra, Vishal Bhardwaj                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>
#include <algorithm>

namespace Belle2 {
  namespace CheckDecay {

    /**
     * Check if two vectors are the same
     */
    inline bool CheckDecay(std::vector<int> MCDAU, int a0 = -987656789,
                           int a1 = -987656789,  int a2 = -987656789,
                           int a3 = -987656789,  int a4 = -987656789,
                           int a5 = -987656789,  int a6 = -987656789,
                           int a7 = -987656789,  int a8 = -987656789,
                           int a9 = -987656789,  int a10 = -987656789,
                           int a11 = -987656789,  int a12 = -987656789,
                           int a13 = -987656789,  int a14 = -987656789,
                           int a15 = -987656789,  int a16 = -987656789,
                           int a17 = -987656789,  int a18 = -987656789)
    {
      std::vector<int> genDAU;
      if (a0 != -987656789) genDAU.push_back(a0);
      if (a1 != -987656789) genDAU.push_back(a1);
      if (a2 != -987656789) genDAU.push_back(a2);
      if (a3 != -987656789) genDAU.push_back(a3);
      if (a4 != -987656789) genDAU.push_back(a4);
      if (a5 != -987656789) genDAU.push_back(a5);
      if (a6 != -987656789) genDAU.push_back(a6);
      if (a7 != -987656789) genDAU.push_back(a7);
      if (a8 != -987656789) genDAU.push_back(a8);
      if (a9 != -987656789) genDAU.push_back(a9);
      if (a10 != -987656789) genDAU.push_back(a10);
      if (a11 != -987656789) genDAU.push_back(a11);
      if (a12 != -987656789) genDAU.push_back(a12);
      if (a13 != -987656789) genDAU.push_back(a13);
      if (a14 != -987656789) genDAU.push_back(a14);
      if (a15 != -987656789) genDAU.push_back(a15);
      if (a16 != -987656789) genDAU.push_back(a16);
      if (a17 != -987656789) genDAU.push_back(a17);
      if (a18 != -987656789) genDAU.push_back(a18);
      std::sort(MCDAU.begin(), MCDAU.end());
      std::sort(genDAU.begin(), genDAU.end());
      return MCDAU == genDAU;
    }
  } // end namespace CheckDecay
} // end namespace Belle2
