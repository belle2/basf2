/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/trackFilter/ParallelTrackFilterModule.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/HitPatternVXD.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <framework/datastore/StoreArray.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ParallelTrackFilter);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ParallelTrackFilterModule::ParallelTrackFilterModule() : Module()
{
  // Set module properties
  setDescription("Generates a new StoreArray from the input StoreArray which contains only tracks that meet the specified criteria.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Parameter definitions
  addParam("inputArrayName", m_inputArrayName, "StoreArray with the input tracks", m_inputArrayName);
  addParam("outputINArrayName", m_outputINArrayName, "Output StoreArray with the tracks that pass the cuts", m_outputINArrayName);
  addParam("outputOUTArrayName", m_outputOUTArrayName, "Output StoreArray with the tracks that do not pass the cuts",
           m_outputOUTArrayName);

  //selection parameter definition
  addParam("min_d0", m_minD0, "minimum value of the d0", m_minD0);
  addParam("max_d0", m_maxD0, "maximum value of the d0", m_maxD0);
  addParam("min_z0", m_minZ0, "minimum value of the z0", m_minZ0);
  addParam("max_z0", m_maxZ0, "maximum value of the z0", m_maxZ0);
  addParam("min_pCM", m_minPCM, "minimum value of the center-of-mass-momentum", m_minPCM);
  addParam("min_pT", m_minPT, "minimum value of the transverse momentum", m_minPT);
  addParam("min_Pvalue", m_minPval, "minimum value of the P-Value of the track fit", m_minPval);
  addParam("min_NumHitPXD", m_minNumHitsPXD, "minimum number of PXD hits associated to the trcak", m_minNumHitsPXD);
  addParam("min_NumHitSVD", m_minNumHitsSVD, "minimum number of SVD hits associated to the trcak", m_minNumHitsSVD);
  addParam("min_NumHitCDC", m_minNumHitsCDC, "minimum number of CDC hits associated to the trcak", m_minNumHitsCDC);

}


void ParallelTrackFilterModule::initialize()
{
  B2DEBUG(22, "ParallelTrackFilterModule " + getName() + " parameters:"
          << LogVar("inputArrayName", m_inputArrayName)
          << LogVar("outputINArrayName", m_outputINArrayName)
          << LogVar("outputOUTArrayName", m_outputOUTArrayName));

  // Can't initialize if the input array is not present
  StoreArray<Track> inputArray(m_inputArrayName);
  if (!inputArray.isOptional()) {
    B2WARNING("Missing input tracks array, " + getName() + " is skipped"
              << LogVar("inputArrayName", m_inputArrayName));
    return;
  }

  m_selectedTracks.registerSubset(inputArray, m_outputINArrayName);
  m_selectedTracks.inheritAllRelations();

  m_notSelectedTracks.registerSubset(inputArray, m_outputOUTArrayName);
  m_notSelectedTracks.inheritAllRelations();
}


void ParallelTrackFilterModule::event()
{

  StoreArray<Track> inputArray(m_inputArrayName);
  if (!inputArray.isOptional() || !m_selectedTracks.getSet()) {
    B2DEBUG(22, "Missing Tracks array, " + getName() + " is skipped." << LogVar("inputArrayName", m_inputArrayName));
    return;
  }

  m_selectedTracks.select([this](const Track * track) {
    return this->isSelected(track);
  });

  m_notSelectedTracks.select([this](const Track * track) {
    return !this->isSelected(track);
  });

}

bool ParallelTrackFilterModule::isSelected(const Track* track)
{
  const TrackFitResult* tfr = track->getTrackFitResultWithClosestMass(Const::pion);
  if (tfr == nullptr)
    return false;

  if (tfr->getD0() < m_minD0 || tfr->getD0() > m_maxD0)
    return false;

  if (tfr->getZ0() < m_minZ0 || tfr->getZ0() > m_maxZ0)
    return false;

  if (tfr->getPValue() < m_minPval)
    return false;

  if (tfr->getMomentum().Rho() < m_minPT)
    return false;

  HitPatternVXD hitPatternVXD = tfr->getHitPatternVXD();
  if (hitPatternVXD.getNSVDHits() < m_minNumHitsSVD ||  hitPatternVXD.getNPXDHits() < m_minNumHitsPXD)
    return false;

  HitPatternCDC hitPatternCDC = tfr->getHitPatternCDC();
  if (hitPatternCDC.getNHits() < m_minNumHitsCDC)
    return false;

  return true;
}
