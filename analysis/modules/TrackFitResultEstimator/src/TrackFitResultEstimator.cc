/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/TrackFitResultEstimator/TrackFitResultEstimator.h>

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
REG_MODULE(TrackFitResultEstimator);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrackFitResultEstimatorModule::TrackFitResultEstimatorModule() : Module()
{
  // Set module properties
  setDescription("Create a TrackFitResult from a Particle's momentum and make a relation between them."); // more information should be added.

  // Parameter definitions
  addParam("inputListName", m_inputListName,
           "The name of input ParticleList.",
           std::string(""));
}

void TrackFitResultEstimatorModule::initialize()
{
  DecayDescriptor decaydescriptor;
  bool valid = decaydescriptor.init(m_inputListName);
  if (!valid)
    B2ERROR("Invalid input ParticleList name: " << m_inputListName);

  const int pdg = decaydescriptor.getMother()->getPDGCode();
  if (abs(TDatabasePDG::Instance()->GetParticle(pdg)->Charge()) > 3)
    B2WARNING("The absolute value of charge of input ParticleList is grater than 1. Helix requires abs(charge) <= 1. "
              "The sign of charge will be used instead.");

  m_inputparticleList.isRequired(m_inputListName);
  m_particles.registerRelationTo(m_trackfitresults);
}

void TrackFitResultEstimatorModule::event()
{

  TMatrixDSym dummyCovariance(6);
  for (int row = 0; row < 6; ++row) {
    dummyCovariance(row, row) = 10000;
  }

  XYZVector position;
  if (m_beamSpotDB)
    position = XYZVector(m_beamSpotDB->getIPPosition().X(), m_beamSpotDB->getIPPosition().Y(), m_beamSpotDB->getIPPosition().Z());
  else
    position = XYZVector(0, 0, 0);

  const double bfield = BFieldManager::getFieldInTesla(position).Z();

  for (unsigned i = 0; i < m_inputparticleList->getListSize(); i++) {
    Particle* part = m_inputparticleList->getParticle(i);

    if (part->getTrack() or part->getTrackFitResult())
      B2ERROR("Particle is already related to the Track or TrackFitResult object.");

    int charge = 0;
    if (part->getCharge() != 0)
      charge = (part->getCharge() > 0) ? 1 : -1;

    TrackFitResult* trkfit = m_trackfitresults.appendNew(position,
                                                         part->getMomentum(),
                                                         dummyCovariance,
                                                         charge,
                                                         Const::ParticleType(part->getPDGCode()),
                                                         -1, // pValue
                                                         bfield,
                                                         0, 0, // CDC, VXD hit-pattern
                                                         0 // NDF
                                                        );
    part->addRelationTo(trkfit);
  }

}
