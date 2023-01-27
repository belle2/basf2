/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <ecl/dataobjects/ECLTrig.h>
#include <framework/datastore/StoreArray.h>
#include <ecl/mapper/ECLChannelMapper.h>

using namespace Belle2;

ECLTrig* ECLTrig::getByCellID(int cid)
{
  static StoreArray<ECLTrig> ecl_trig_arr;
  static auto mapper = ECL::ECLChannelMapper::getInstance();
  int crate = mapper->getCrateID(cid);

  if (crate < 1 || crate > 52) {
    B2FATAL("Could not determine crate ID for cid " << cid);
  }

  for (auto& ecl_trig : ecl_trig_arr) {
    if (ecl_trig.getTrigId() == crate) return &ecl_trig;
  }

  return nullptr;
}

