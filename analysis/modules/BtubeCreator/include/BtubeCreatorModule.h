/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sourav Dey, Abi Soffer                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>


// DataStore
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// DataObjects
#include <mdst/dbobjects/BeamSpot.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Btube.h>

// rave
#include <analysis/VertexFitting/RaveInterface/RaveSetup.h>
#include <analysis/VertexFitting/RaveInterface/RaveVertexFitter.h>
#include <analysis/VertexFitting/RaveInterface/RaveKinematicVertexFitter.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>
namespace Belle2 {
  /**
   * Create a B particle from a Bbar particle
   *
   * set the direction of the recoil B   *
   */
  class BtubeCreatorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    BtubeCreatorModule();

    /** declare data store elements */
    virtual void initialize() override;

    /** process event */
    virtual void event() override;

  private:

    std::string m_listName;  /**< name of particle list */
    std::string m_decayString;          /**< specifies which daughter particles will be used as reference to create Btube*/
    bool m_associateBtubeToBselected; /**< whether to associate the Btube with the selected B*/
    double m_confidenceLevel;     /**< required fit confidence level */
    double m_Bfield;              /**< magnetic field from data base */
    TVector3 m_BeamSpotCenter;    /**< Beam spot position */
    TMatrixDSym m_beamSpotCov;    /**< Beam spot covariance matrix */
    DBObjPtr<BeamSpot> m_beamSpotDB;/**< Beam spot database object */
    StoreArray<Btube> m_tubeArray; /**< the (output) array of Btube objects */
    StoreObjPtr<ParticleList> m_plist; /**< the input particle list */
    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of decays to look for. */
    /**
     * Main steering routine
     * @param p pointer to particle
     * @return true for successful fit and prob(chi^2,ndf) > m_confidenceLevel
     */
    bool doVertexFit(Particle* p);  /**< does the vertex fit with RAVE :  adaptive vertex fitter mode */
    void addextrainfos(Particle* daughter, Particle* copy, TMatrix mat, TLorentzVector TLV); /**< fills extrainfos to the particle */
    bool m_verbose;  /**< run fit with a lot of B2INFOs for debugging */
  };
}
