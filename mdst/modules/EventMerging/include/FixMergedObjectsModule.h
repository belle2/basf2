/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/dataobjects/EventExtraInfo.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/V0.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/core/Module.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <map>
#include <string>

namespace Belle2 {

  /**
   * If the content of two DataStores are merged using the 'MergeDataStoreModule',
   * then Relations of the objects are corrected.
   * BUT: some of the mdst objects internally store Relations as the Indices of
   * the objects in the StoreArray. This module fixes these internal relations for
   * the standard mdst objects.
   */
  class FixMergedObjectsModule : public Module {

  public:

    /**
     * Constructor
     */
    FixMergedObjectsModule();

    /**
     * Initialize the Module.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  private:

    StoreObjPtr<EventExtraInfo> m_mergedArrayIndices; /**< indices where the StoreArrays were merged */
    StoreArray<Track> m_tracks; /**< tracks */
    StoreArray<V0> m_v0s; /**< v0s */
    StoreArray<MCParticle> m_mcParticles; /**< mcparticles */
  };

} // Belle2 namespace

