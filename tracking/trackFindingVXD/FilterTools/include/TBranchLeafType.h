/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <Rtypes.h>

namespace Belle2 {

  char
  TBranchLeafType(const char* var) { return 'C'; };

  char
  TBranchLeafType(const Char_t& var) { return 'B'; };

  char
  TBranchLeafType(const unsigned char& var) { return 'b'; };

  char
  TBranchLeafType(const short& var) { return 'S'; };

  char
  TBranchLeafType(const unsigned short& var) { return 's'; };

  char
  TBranchLeafType(const Int_t& var) { return 'I'; };

  char
  TBranchLeafType(const UInt_t& var) { return 'i'; };

  char
  TBranchLeafType(const Float_t& var) { return 'F'; };

  char
  TBranchLeafType(const Double_t& var) { return 'D'; };

  char
  TBranchLeafType(const long int& var) { return 'L'; };

  char
  TBranchLeafType(const unsigned long int& var) { return 'l'; };

  char
  TBranchLeafType(const bool& var) { return 'O'; };


}
