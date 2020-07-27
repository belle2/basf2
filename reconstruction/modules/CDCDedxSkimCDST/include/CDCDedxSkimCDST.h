/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <reconstruction/dataobjects/DedxConstants.h>
#include <reconstruction/dataobjects/CDCDedxTrack.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/core/Module.h>
#include <framework/datastore/SelectSubset.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>

#include <string>
#include <vector>

namespace Belle2 {

  /** Extracts dE/dx information for calibration testing. Writes a ROOT file.
   */
  class CDCDedxSkimCDSTModule : public Module {

  public:

    /** Default constructor */
    CDCDedxSkimCDSTModule();

    /** Destructor */
    virtual ~CDCDedxSkimCDSTModule();

    /** Initialize the module */
    virtual void initialize() override;

    /** Selection function to skim DataStore */
    //    void mySelectionFunction(const CDCDedxTrack* dedxTrack);

    /** This method is called for each event. All processing of the event
     * takes place in this method. */
    virtual void event() override;

    /** End of the event processing. */
    virtual void terminate() override;

  private:
    bool isRecList;/**< set particle list type FS or reco particle list */
    std::vector<std::string> m_strParticleList; /**< Vector of ParticleLists to write out */
    StoreArray<CDCDedxTrack> m_dedxTracks; /**< Intput array of CDCDedxTracks */
    SelectSubset<CDCDedxTrack> m_selector; /**< Used to get a subset of the input array */
  };
} // Belle2 namespace
