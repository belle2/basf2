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

  inline char
  TBranchLeafType(const char*) { return 'C'; };

  inline char
  TBranchLeafType(const Char_t&) { return 'B'; };

  inline char
  TBranchLeafType(const unsigned char&) { return 'b'; };

  inline char
  TBranchLeafType(const short&) { return 'S'; };

  inline char
  TBranchLeafType(const unsigned short&) { return 's'; };

  inline char
  TBranchLeafType(const Int_t&) { return 'I'; };

  inline char
  TBranchLeafType(const UInt_t&) { return 'i'; };

  inline char
  TBranchLeafType(const Float_t&) { return 'F'; };

  inline char
  TBranchLeafType(const Double_t&) { return 'D'; };

  inline char
  TBranchLeafType(const long int&) { return 'L'; };

  inline char
  TBranchLeafType(const unsigned long int&) { return 'l'; };

  inline char
  TBranchLeafType(const bool&) { return 'O'; };


}
