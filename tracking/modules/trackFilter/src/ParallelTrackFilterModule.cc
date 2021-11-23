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

double ParallelTrackFilterModule::m_min_d0 = -100;
double ParallelTrackFilterModule::m_max_d0 = +100;
double ParallelTrackFilterModule::m_min_z0 = -500;
double ParallelTrackFilterModule::m_max_z0 = +500;
int ParallelTrackFilterModule::m_min_NumHitsSVD = 0;
int ParallelTrackFilterModule::m_min_NumHitsPXD = 0;
int ParallelTrackFilterModule::m_min_NumHitsCDC = 0;
double ParallelTrackFilterModule::m_min_pCM = 0;
double ParallelTrackFilterModule::m_min_pT = 0;
double ParallelTrackFilterModule::m_min_Pval = 0;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ParallelTrackFilter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ParallelTrackFilterModule::ParallelTrackFilterModule() : Module()
{
  // Set module properties
  setDescription("Generates a new StoreArray from the input StoreArray which has all specified Tracks removed.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Parameter definitions
  addParam("inputArrayName", m_inputArrayName, "StoreArray with the input tracks", std::string("Tracks"));
  addParam("outputINArrayName", m_outputINArrayName, "StoreArray with the output tracks", std::string(""));
  addParam("outputOUTArrayName", m_outputOUTArrayName, "StoreArray with the output tracks", std::string(""));

  //selection parameter definition
  addParam("min_d0", m_min_d0, "minimum value of the d0", double(-100));
  addParam("max_d0", m_max_d0, "maximum value of the d0", double(+100));
  addParam("min_z0", m_min_z0, "minimum value of the z0", double(-500));
  addParam("max_z0", m_max_z0, "maximum value of the z0", double(+500));
  addParam("min_pCM", m_min_pCM, "minimum value of the center-of-mass-momentum", double(0));
  addParam("min_pT", m_min_pT, "minimum value of the transverse momentum", double(0));
  addParam("min_Pvalue", m_min_Pval, "minimum value of the P-Value of the track fit", double(0));
  addParam("min_NumHitPXD", m_min_NumHitsPXD, "minimum number of PXD hits associated to the trcak", int(0));
  addParam("min_NumHitSVD", m_min_NumHitsSVD, "minimum number of SVD hits associated to the trcak", int(0));
  addParam("min_NumHitCDC", m_min_NumHitsCDC, "minimum number of CDC hits associated to the trcak", int(0));

}


void ParallelTrackFilterModule::initialize()
{
  B2INFO("ParallelTrackFilterModule " + getName() + " parameters:"
         << LogVar("inputArrayName", m_inputArrayName)
         << LogVar("outputINArrayName", m_outputINArrayName)
         << LogVar("outputOUTArrayName", m_outputOUTArrayName));

  initializeSelectSubset();
}


void ParallelTrackFilterModule::beginRun()
{
  initializeSelectSubset();
}


void ParallelTrackFilterModule::initializeSelectSubset()
{

  // Attepmt SelectSubset<Track> initialization if not done already
  if (m_selectedTracks.getSet())
    return;

  // Can't initialize if the input array is not present (may change from run to run)
  StoreArray<Track> inputArray(m_inputArrayName);
  if (!inputArray.isOptional())
    return;

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

  m_selectedTracks.select(isSelected);

  m_notSelectedTracks.select([](const Track * track) {
    return !isSelected(track);
  });

}

bool ParallelTrackFilterModule::isSelected(const Track* track)
{

  bool isExcluded = false;
  int pionCode = 211;

  const TrackFitResult*  tfr = track->getTrackFitResult(Const::ChargedStable(pionCode));
  if (tfr == nullptr)
    return false;

  if (tfr->getD0() < m_min_d0 || tfr->getD0() > m_max_d0)
    isExcluded = true;

  if (tfr->getZ0() < m_min_z0 || tfr->getZ0() > m_max_z0)
    isExcluded = true;

  if (tfr->getPValue() < m_min_Pval)
    isExcluded = true;

  if (tfr->getMomentum().Perp() < m_min_pT)
    isExcluded = true;

  HitPatternVXD hitPatternVXD = tfr->getHitPatternVXD();
  if (hitPatternVXD.getNSVDHits() < m_min_NumHitsSVD ||  hitPatternVXD.getNPXDHits() < m_min_NumHitsPXD)
    isExcluded = true;

  HitPatternCDC hitPatternCDC = tfr->getHitPatternCDC();
  if (hitPatternCDC.getNHits() < m_min_NumHitsCDC)
    isExcluded = true;

  return !isExcluded;

}
