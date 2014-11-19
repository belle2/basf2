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
  TBranchLeafType(const char*) { return 'C'; };

  char
  TBranchLeafType(const Char_t&) { return 'B'; };

  char
  TBranchLeafType(const unsigned char&) { return 'b'; };

  char
  TBranchLeafType(const short&) { return 'S'; };

  char
  TBranchLeafType(const unsigned short&) { return 's'; };

  char
  TBranchLeafType(const Int_t&) { return 'I'; };

  char
  TBranchLeafType(const UInt_t&) { return 'i'; };

  char
  TBranchLeafType(const Float_t&) { return 'F'; };

  char
  TBranchLeafType(const Double_t&) { return 'D'; };

  char
  TBranchLeafType(const long int&) { return 'L'; };

  char
  TBranchLeafType(const unsigned long int&) { return 'l'; };

  char
  TBranchLeafType(const bool&) { return 'O'; };


}
