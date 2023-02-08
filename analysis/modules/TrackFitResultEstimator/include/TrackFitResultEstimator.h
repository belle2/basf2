/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dbobjects/BeamSpot.h>


#include <string>

namespace Belle2 {
  /**
  * Create a TrackFitResult from the momentum of a Particle and make a relation between them.
  */
  class TrackFitResultEstimatorModule : public Module {

  public:

    /**
    * Constructor
    */
    TrackFitResultEstimatorModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  private:

    std::string m_inputListName;  /**< The name of input ParticleList */
    StoreObjPtr<ParticleList>  m_inputparticleList; /**< StoreObjptr for input charged ParticleList */
    StoreArray<Particle> m_particles; /**< StoreArray of Particle objects */
    StoreArray<TrackFitResult> m_trackfitresults; /**< StoreArray of TrackFitResult objects */

    DBObjPtr<BeamSpot> m_beamSpotDB; /**< Beam spot database object */

  };
}

