/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/vxdCaTracking/XHitFilterFactory.h>
#include <tracking/trackFindingVXD/sectorMapTools/SecMapTrainer.h>
#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <framework/logging/Logger.h>

#include <string>
#include <vector>

namespace Belle2 {
  /** The SecMapTrainerVXDTFModule
   * this module analyzes a big number of events (pGun or evtGen) to create raw sectorMaps which are needed for the VXDTF.
   * This information will be exported via and root files.
   */
  class SecMapTrainerVXDTFModule : public Module {

  public:

    /** SecMapTrainerVXDTFModule constructor. */
    SecMapTrainerVXDTFModule();

    /** SecMapTrainerVXDTFModule destructor. */
    ~SecMapTrainerVXDTFModule() {}

    /** initialize. */
    void initialize() override;

    /** beginRun. */
    void beginRun() override
    { B2INFO("~~~~~~~~~~~SecMapTrainerVXDTFModule - beginRun ~~~~~~~~~~"); }

    /** event. */
    void event() override;

    /** endRun. */
    void endRun() override
    { B2INFO("~~~~~~~~~~~SecMapTrainerVXDTFModule - end of endRun ~~~~~~~~~~"); }

    /** terminate. */
    void terminate() override;

    /** initialize variables to avoid nondeterministic behavior. */
    void InitializeVariables() {}

  protected:
    /** Event Data for distinguishing events. */
    StoreObjPtr<EventMetaData> m_eventData;

    /** contains the spacePointTrackCands to be analyzed for the secMap-Training. */
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;

    /** contains the trainers for the secMaps to be trained. */
    std::vector< SecMapTrainer<XHitFilterFactory<SecMapTrainerHit> > > m_secMapTrainers;

    /** Name of storeArray containing the spacePointTrackCands. */
    std::string m_PARAMspTCarrayName;

    /// If true, training will be executed and filled into rootFiles,
    /// if not, only things like basf2 -m work but no training can be done.
    bool m_PARAMallowTraining;

    // TODO:
    ///** Name of storeArray containing the spacePointTrackCands. */
    //std::vector<std::string> m_PARAMsecMapNames;

    ///** secMap-specific configurations for training (musst have same size as m_PARAMsecMapNames. */
    //std::vector<TrainerConfigData> m_PARAMsecMapConfigs;
  };
}
