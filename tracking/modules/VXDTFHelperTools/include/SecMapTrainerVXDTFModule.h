/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/vxdCaTracking/XHitFilterFactory.h>
#include <tracking/trackFindingVXD/sectorMapTools/SecMapTrainer.h>
#include <tracking/trackFindingVXD/sectorMapTools/TrainerConfigData.h>
#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

// #include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// #include <iostream>
// #include <fstream>
#include <string>
#include <vector>
// #include <list>
#include <map>
#include <utility> // pair

#include <TVector3.h>
#include <TTree.h>
#include <TFile.h>


namespace Belle2 {



  /** The SecMapTrainerVXDTFModule
   *
   * this module analyzes a big number of events (pGun or evtGen) to create raw sectorMaps which are needed for the VXDTF.
   * This information will be exported via and root files.
   *
  . */
  class SecMapTrainerVXDTFModule : public Module {


  public:

    /** SecMapTrainerVXDTFModule constructor. */
    SecMapTrainerVXDTFModule();

    /** SecMapTrainerVXDTFModule destructor. */
    virtual ~SecMapTrainerVXDTFModule() { delete m_testRootfile; }

    /** initialize. */
    virtual void initialize();

    /** beginRun. */
    virtual void beginRun()
    { B2INFO("~~~~~~~~~~~SecMapTrainerVXDTFModule - beginRun ~~~~~~~~~~") }

    /** event. */
    virtual void event();

    /** endRun. */
    virtual void endRun()
    { B2INFO("~~~~~~~~~~~SecMapTrainerVXDTFModule - end of endRun ~~~~~~~~~~") }

    /** terminate. */
    virtual void terminate();

    /** initialize variables to avoid nondeterministic behavior. */
    void InitializeVariables() {}





  protected:

    /** Event Data for distinguishing events. */
    StoreObjPtr<EventMetaData> m_eventData;

    /** contains the spacePointTrackCands to be analyzed for the secMap-Training. */
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;

    TFile* m_testRootfile;
    TrainerConfigData m_testConfig;

    SecMapTrainer<XHitFilterFactory<SecMapTrainerHit>> m_testTrainer;


    /** Name of storeArray containing the spacePointTrackCands. */
    std::string m_PARAMspTCarrayName;
  private:
  };
}
