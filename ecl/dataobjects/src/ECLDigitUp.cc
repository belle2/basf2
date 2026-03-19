/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <ecl/dataobjects/ECLDigitUp.h>
#include <framework/datastore/StoreArray.h>

using namespace Belle2;

ECLDigitUp* ECLDigitUp::getByCellID(int cid)
{
  static StoreArray<ECLDigitUp> ecl_digits_arr;
  for (auto& ecl_dig : ecl_digits_arr) {
    if (ecl_dig.getCellId() == cid) return &ecl_dig;
  }
  return nullptr;
}

