/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Klemens Lautenbach                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/modules/PseudoVertexFitter/PseudoVertexFitterModule.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/dbobjects/BeamParameters.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

// utilities
#include <analysis/utility/CLHEPToROOT.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ParticleCopy.h>

// Magnetic field
#include <framework/geometry/BFieldManager.h>

#include <TMath.h>

using namespace std;

namespace Belle2 {


  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(PseudoVertexFitter)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  PseudoVertexFitterModule::PseudoVertexFitterModule() : Module()
  {
    // set module description (e.g. insert text)
    setDescription("Pseudo fitter adds a covariance matrix which is sum of the daughter covariance matrices.");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("listName", m_listName, "name of particle list", string(""));
    addParam("decayString", m_decayString, "specifies which daughter particles are included in the kinematic fit", string(""));
  }

  void PseudoVertexFitterModule::initialize()
  {
    B2INFO("PseudoVertexFitter: adding covariance matrix to " << m_listName);
    if (m_decayString != "")
      B2INFO("PseudoVertexFitter: Using specified decay string: " << m_decayString);
  }

  void PseudoVertexFitterModule::beginRun()
  {

  }

  void PseudoVertexFitterModule::event()
  {

    StoreObjPtr<ParticleList> plist(m_listName);
    if (!plist) {
      B2ERROR("ParticleList " << m_listName << " not found");
      return;
    }

    std::vector<unsigned int> toRemove;
    unsigned int n = plist->getListSize();
    for (unsigned i = 0; i < n; i++) {
      Particle* particle = plist->getParticle(i);


      bool ok = add_matrix(particle);

      if (!ok) particle->setPValue(-1);
      if (particle->getPValue() == 0.) {
        toRemove.push_back(particle->getArrayIndex());
      } else {
        if (particle->getPValue() < 0)toRemove.push_back(particle->getArrayIndex());
      }
    }
    plist->removeParticles(toRemove);
  }

  bool PseudoVertexFitterModule::add_matrix(Particle* mother)
  {
    // steering starts here
    if (mother->getNDaughters() < 2) return false;

    bool ok = false;

    const std::vector<Particle*> daughters = mother->getDaughters();
    std::vector<TMatrixFSym> daughter_matrices;
    for (unsigned int i = 0; i < daughters.size(); i++) {
      daughter_matrices.push_back(daughters[i]->getMomentumVertexErrorMatrix());
    }
    TMatrixFSym mother_errMatrix(7);
    for (int i = 0; i < 7; i++) {
      for (int j = 0; j < 7; j++) {
        for (unsigned int k = 0; k < daughters.size(); k++) {
          mother_errMatrix[i][j] += daughter_matrices[k][i][j];
        }
      }
    }
    mother->setMomentumVertexErrorMatrix(mother_errMatrix);
    if (mother->getMomentumVertexErrorMatrix() == mother_errMatrix) {
      ok = true;
      B2ERROR("Matrix added succesfully!");
    }
//  if(mother->getPValue() > 0) {
//    ok=true;
//    B2ERROR("");
//  }
    if (!ok) return false;
//
//    //if (mother->getPValue() < m_confidenceLevel) return false;
    return true;
  }
} // end Belle2 namespace

