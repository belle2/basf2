/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/PseudoVertexFitter/PseudoVertexFitterModule.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <framework/database/DBObjPtr.h>

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
  }

  void PseudoVertexFitterModule::initialize()
  {
    if (m_listName == "") {
      B2ERROR("No list name specified! Please enter the list name of the particle you want to add a covariance matrix to.");
      return;
    }
    B2INFO("PseudoVertexFitter: adding covariance matrix to " << m_listName);

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
    }
  }

  bool PseudoVertexFitterModule::add_matrix(Particle* mother)
  {
    // steering starts here
    if (mother->getNDaughters() < 2) return false;

    bool ok = false;

    const std::vector<Particle*> daughters = mother->getDaughters();
    std::vector<TMatrixFSym> daughter_matrices;
    daughter_matrices.reserve(daughters.size());
    for (auto daughter : daughters) {
      daughter_matrices.push_back(daughter->getMomentumVertexErrorMatrix());
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
    }
    if (!ok) return false;
    return true;
  }
} // end Belle2 namespace

