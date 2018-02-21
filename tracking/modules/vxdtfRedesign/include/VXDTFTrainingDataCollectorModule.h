/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Eugenio Paoloni                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/trackFindingVXD/filterTools/SelectionVariableFactory.h>
#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <string>
#include <vector>




namespace Belle2 {


  /** The VXDTFTrainingDataCollectorModule
   *
   * this module collect from the data needed to train the
   * VXDTF sector map. The data are saved on a root file that eventually
   * will be chained and used by the training module.
   *
  . */
  class VXDTFTrainingDataCollectorModule : public Module {


  public:
    /** VXDTFTrainingDataCollectorModule constructor. */
    VXDTFTrainingDataCollectorModule();

    /** VXDTFTrainingDataCollectorModule destructor. */
    ~VXDTFTrainingDataCollectorModule() {}

    /** initialize. */
    void initialize() override;

    /** beginRun. */
    void beginRun() override
    { }

    /** event. */
    void event() override;

    /** endRun. */
    void endRun() override
    { }

    /** terminate. */
    void terminate() override;

  protected:

    /** Event meta data of the event */
    StoreObjPtr<EventMetaData> m_eventData;

    /** spacePointTrackCands to be digested. */
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;

    /** Name of storeArray containing the spacePointTrackCands. */
    std::string m_PARAMSpacePointTrackCandsName;

    /** Name tag that will be attached to the output file*/
    std::string m_PARAMNameTag;

    /** Names of the variables to be produced and collected */
    std::vector <
    SecMapTrainer<SelectionVariableFactory<SecMapTrainerHit> > >
    m_secMapTrainers;


  private:
  };
}
