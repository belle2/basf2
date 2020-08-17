/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sourav Patra, Vishal Bhardwaj                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <TTree.h>
#include <string>
#include <vector>

namespace Belle2 {

  /** NtupleTool to write the  decay mode ID generated. */
  class GenBplusTag {
  private:
    int nPhotos;
    int Array_CheckDecay(std::vector<int> MCDAU, std::vector<int> genDAU);
    int CheckDecay(std::vector<int>MCDAU, int a0, int a1, int a2, int a3, int a4,
                   int a5, int a6, int a7, int a8, int a9, int a10, int a11, int a12,
                   int a13, int a14, int a15, int a16, int a17, int a18);
    int DauSize(std::vector<int>genpart);
    int PcheckDecay(std::vector<int>gp, int da1, int da2);
    int PcheckDecay(std::vector<int>gp, int da1, int da2, int da3);
    int PcheckDecay(std::vector<int>gp, int da1, int da2, int da3, int da4);
    int PcheckDecay(std::vector<int>gp, int da1, int da2, int da3, int da4, int da5);
    int PcheckDecay(std::vector<int>gp, int da1, int da2, int da3, int da4, int da5,
                    int da6);
    int PcheckDecay(std::vector<int>gp, int da1, int da2, int da3, int da4, int da5,
                    int da6, int da7);
    int PcheckDecay(std::vector<int>gp, int da1, int da2, int da3, int da4, int da5,
                    int da6, int da7, int da8);
  public:
    int Mode_B_plus(std::vector<int> genDAU);
    int Mode_B_minus(std::vector<int> genDAU);
  };

} //End of Belle2 namespace