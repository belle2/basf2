/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/TrackHelixEstimator/TrackHelixEstimator.h>

#include <framework/datastore/RelationArray.h>
#include <framework/geometry/BFieldManager.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/ClusterUtility/ClusterUtils.h>

#include <TDatabasePDG.h>
#include <Math/Vector3D.h>
#include <vector>

using namespace Belle2;
using namespace ROOT::Math;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TrackHelixEstimator);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrackHelixEstimatorModule::TrackHelixEstimatorModule() : Module()
{
  // Set module properties
  setDescription("Create a TrackFitResult from a Particle's momentum and make a relation between them."); // more information should be added.

  // Parameter definitions
  addParam("inputListName", m_inputListName,
           "The name of input charged ParticleList. Only the charged particle is available.",
           std::string(""));
}

void TrackHelixEstimatorModule::initialize()
{
  DecayDescriptor decaydescriptor;
  bool valid = decaydescriptor.init(m_inputListName);
  if (!valid)
    B2ERROR("Invalid input ParticleList name: " << m_inputListName);

  const int pdg = decaydescriptor.getMother()->getPDGCode();
  if (TDatabasePDG::Instance()->GetParticle(pdg)->Charge() == 0)
    B2ERROR("The input ParticleList is for a neutral particle. The charged particle is required.");

  m_inputparticleList.isRequired(m_inputListName);
  m_particles.registerRelationTo(m_trackfitresults);
}

void TrackHelixEstimatorModule::event()
{

  TMatrixDSym dummyCovariance(7);
  for (int row = 0; row < 7; ++row) {
    dummyCovariance(row, row) = 10000;
  }

  ClusterUtils cUtil;
  const XYZVector IPPosition = cUtil.GetIPPosition(); // do we want to use another position?

  const double bfield = BFieldManager::getFieldInTesla(IPPosition).Z();

  for (unsigned i = 0; i < m_inputparticleList->getListSize(); i++) {
    Particle* part = m_inputparticleList->getParticle(i);

    if (part->getTrack() or part->getTrackFitResult())
      B2ERROR("Particle is already related to the Track or TrackFitResult object.");

    TrackFitResult* trkfit = m_trackfitresults.appendNew(IPPosition,
                                                         part->getMomentum(),
                                                         dummyCovariance,
                                                         part->getCharge(),
                                                         Const::ParticleType(part->getPDGCode()),
                                                         -1, // pValue
                                                         bfield,
                                                         0, 0, // CDC, VXD hit-pattern
                                                         0 // NDF
                                                        );
    part->addRelationTo(trkfit);
  }

}
