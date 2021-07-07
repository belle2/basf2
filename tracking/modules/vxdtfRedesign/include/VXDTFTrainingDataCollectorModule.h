/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
