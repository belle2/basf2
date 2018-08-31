/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 * Contributors: Patrick Ecker                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>

namespace Belle2 {
  /**
   * Class to save found bremsstrahlung photons
   */
  class BremPhoton : public RelationsObject {
  public:
    /**
     * Default constructor
     */
    BremPhoton() = default;

    BremPhoton(const Track* track, const ECLCluster* bremCluster, double accFactor)
    {
      addRelationTo(track, accFactor);
      addRelationTo(bremCluster, accFactor);
    }

    ClassDef(BremPhoton, 1);
  };
}
