/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ContinuumSuppression.h>

#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <mdst/dbobjects/BeamSpot.h>

#include <string>

namespace Belle2 {
  class Particle;
  /**
   * Creates for each Particle in given ParticleList an ContinuumSuppression
   * dataobject and makes basf2 relation between them.
   */
  class ContinuumSuppressionBuilderModule : public Module {
  public:

    /** constructor */
    ContinuumSuppressionBuilderModule();
    /** initialize the module (setup the data store) */
    virtual void initialize() override;
    /** process event */
    virtual void event() override;

  private:

    StoreArray<ContinuumSuppression> m_csarray; /**< StoreArray of ContinuumSuppression */
    StoreObjPtr<ParticleList> m_plist;          /**< input particle list */
    DBObjPtr<BeamSpot> m_beamSpotDB;            /**< Beam spot database object */

    ROOT::Math::XYZVector m_BeamSpotCenter;     /**< Beam spot position */
    TMatrixDSym m_beamSpotCov;                  /**< Beam spot covariance matrix */

    std::string m_particleListName;             /**< Name of the ParticleList */
    std::string m_ROEMask;                      /**< ROE mask */
    double m_Bfield;                            /**< magnetic field from data base */
    bool m_ipProfileFit;                        /**< Switch to turn on / off vertex fit of tracks with IP profile constraint */

    /** calculate continuum suppression quantities */
    void addContinuumSuppression(const Particle* particle);

    /** get 4Vector for CS calculation with or without IP profile fit */
    ROOT::Math::PxPyPzEVector ipProfileFit(const Particle* particle);
  };
}
