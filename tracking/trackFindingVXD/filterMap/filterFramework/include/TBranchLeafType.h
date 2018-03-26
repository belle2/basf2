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
  /// Overloading TBranchLeafType to be able to get identifier 'C' for type char*
  inline char TBranchLeafType(const char*) { return 'C'; };

  /// Overloading TBranchLeafType to be able to get identifier 'B' for type Char_t
  inline char TBranchLeafType(const Char_t&) { return 'B'; };

  /// Overloading TBranchLeafType to be able to get identifier 'b' for type unsigned char
  inline char TBranchLeafType(const unsigned char&) { return 'b'; };

  /// Overloading TBranchLeafType to be able to get identifier 'S' for type short
  inline char TBranchLeafType(const short&) { return 'S'; };

  /// Overloading TBranchLeafType to be able to get identifier 's' for type unsigned short
  inline char TBranchLeafType(const unsigned short&) { return 's'; };

  /// Overloading TBranchLeafType to be able to get identifier 'I' for type Int_t
  inline char TBranchLeafType(const Int_t&) { return 'I'; };

  /// Overloading TBranchLeafType to be able to get identifier 'i' for type UInt_t
  inline char TBranchLeafType(const UInt_t&) { return 'i'; };

  /// Overloading TBranchLeafType to be able to get identifier 'F' for type Float_t
  inline char TBranchLeafType(const Float_t&) { return 'F'; };

  /// Overloading TBranchLeafType to be able to get identifier 'D' for type Double_t
  inline char TBranchLeafType(const Double_t&) { return 'D'; };

  /// Overloading TBranchLeafType to be able to get identifier 'L' for type long int
  inline char TBranchLeafType(const long int&) { return 'L'; };

  /// Overloading TBranchLeafType to be able to get identifier 'l' for type unsigned long int
  inline char TBranchLeafType(const unsigned long int&) { return 'l'; };

  /// Overloading TBranchLeafType to be able to get identifier 'O' for type bool
  inline char TBranchLeafType(const bool&) { return 'O'; };
}
