/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <ecl/dataobjects/ECLDsp.h>
#include <framework/datastore/StoreArray.h>

using namespace Belle2;

ECLDsp* ECLDsp::getByCellID(int cid)
{
  static StoreArray<ECLDsp> ecl_dsp_arr;
  for (auto& ecl_dsp : ecl_dsp_arr) {
    if (ecl_dsp.getCellId() == cid) return &ecl_dsp;
  }
  return nullptr;
}

