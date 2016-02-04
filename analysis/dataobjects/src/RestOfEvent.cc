/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc, Matic Lubej                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dataobjects/RestOfEvent.h>

#include <framework/datastore/StoreArray.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <framework/logging/Logger.h>
#include <iostream>

using namespace Belle2;

void RestOfEvent::addTrack(const Track* track)
{
  m_trackIndices.insert(track->getArrayIndex());
}

void RestOfEvent::addTracks(std::vector<int>& indices)
{
  addIndices(indices, m_trackIndices);
}

void RestOfEvent::addECLCluster(const ECLCluster* shower)
{
  m_eclClusterIndices.insert(shower->getArrayIndex());
}

void RestOfEvent::addECLClusters(std::vector<int>& indices)
{
  addIndices(indices, m_eclClusterIndices);
}

void RestOfEvent::addKLMCluster(const KLMCluster* cluster)
{
  m_klmClusterIndices.insert(cluster->getArrayIndex());
}

void RestOfEvent::addKLMClusters(std::vector<int>& indices)
{
  addIndices(indices, m_klmClusterIndices);
}

void RestOfEvent::appendChargedStableFractionsSet(std::map<std::string, std::vector<double>> fractionsSet)
{
  m_fractionsSet.insert(fractionsSet.begin(), fractionsSet.end());
}

void RestOfEvent::appendTrackMasks(std::map<std::string, std::map<unsigned int, bool>> masks)
{
  m_trackMasks.insert(masks.begin(), masks.end());
}

void RestOfEvent::appendECLClusterMasks(std::map<std::string, std::map<unsigned int, bool>> masks)
{
  m_eclClusterMasks.insert(masks.begin(), masks.end());
}

std::vector<Belle2::Track*> RestOfEvent::getTracks(std::string maskName) const
{
  std::vector<Track*> remainTracks(getNTracks(maskName));
  StoreArray<Track> allTracks;

  if (allTracks.getEntries() < getNTracks(maskName))
    B2ERROR("[RestOfEvent::getAllTracks] Number of remaining tracks in the RestOfEvent > number of all tracks in StoreArray<Track>!");

  int i = 0;
  std::map<unsigned int, bool> trackMask = RestOfEvent::getTrackMask(maskName);

  if (trackMask.empty())
    for (const int index : m_trackIndices) {
      remainTracks[i] = allTracks[index];
      i++;
    }

  else
    for (const int index : m_trackIndices) {
      if (trackMask.at(index)) {
        remainTracks[i] = allTracks[index];
        i++;
      }
    }

  return remainTracks;
}

std::vector<Belle2::ECLCluster*> RestOfEvent::getECLClusters(std::string maskName) const
{
  std::vector<ECLCluster*> remainECLClusters(getNECLClusters(maskName));
  StoreArray<ECLCluster> allECLClusters;

  if (allECLClusters.getEntries() < getNECLClusters(maskName))
    B2ERROR("[RestOfEvent::getAllECLClusters] Number of remaining ECL showers in the RestOfEvent > number of all showers in StoreArray<ECLCluster>!");

  int i = 0;
  std::map<unsigned int, bool> eclClusterMask = RestOfEvent::getECLClusterMask(maskName);

  if (eclClusterMask.empty())
    for (const int index : m_eclClusterIndices) {
      remainECLClusters[i] = allECLClusters[index];
      i++;
    }

  else
    for (const int index : m_eclClusterIndices) {
      if (eclClusterMask.at(index)) {
        remainECLClusters[i] = allECLClusters[index];
        i++;
      }
    }

  return remainECLClusters;
}

std::vector<Belle2::KLMCluster*> RestOfEvent::getKLMClusters() const
{
  std::vector<KLMCluster*> remainKLMClusters(getNKLMClusters());
  StoreArray<KLMCluster> allKLMClusters;

  if (allKLMClusters.getEntries() < getNKLMClusters())
    B2ERROR("[RestOfEvent::getAllKLMClusters] Number of remaining KLM clusters in the RestOfEvent > number of all clusters in StoreArray<KLMCluster>!");

  int i = 0;
  for (const int index : m_klmClusterIndices) {
    remainKLMClusters[i] = allKLMClusters[index];
    i++;
  }

  return remainKLMClusters;
}

TLorentzVector RestOfEvent::get4Vector(std::string maskName) const
{
  std::vector<Track*> roeTracks = RestOfEvent::getTracks(maskName);
  std::vector<ECLCluster*> roeClusters = RestOfEvent::getECLClusters(maskName);
  TLorentzVector roe4Vector;

  const unsigned int n = Const::ChargedStable::c_SetSize;
  double fractions[n];
  fillFractions(fractions, maskName);

  // Add all momenta from tracks
  for (unsigned int iTrack = 0; iTrack < roeTracks.size(); iTrack++) {

    const Track* track = roeTracks[iTrack];
    const PIDLikelihood* pid = track->getRelatedTo<PIDLikelihood>();
    const MCParticle* mcp = roeTracks[iTrack]->getRelatedTo<MCParticle>();

    if (!pid) {
      B2ERROR("Track with no PID information!");
      continue;
    }

    int particlePDG = Const::pion.getPDGCode();

    if (fractions[0] == -1) {
      if (mcp)
        particlePDG = abs(mcp->getPDG());
      else
        B2WARNING("No related MCParticle found! Default will be used.");
    } else
      particlePDG = pid->getMostLikely(fractions).getPDGCode();

    Const::ChargedStable trackParticle = Const::ChargedStable(particlePDG);
    const TrackFitResult* tfr = roeTracks[iTrack]->getTrackFitResult(trackParticle);

    // Set energy of track
    float tempMass = trackParticle.getMass();
    TVector3 tempMom = tfr->getMomentum();
    TLorentzVector temp4Vector;
    temp4Vector.SetVect(tempMom);
    temp4Vector.SetE(TMath::Sqrt(tempMom.Mag2() + tempMass * tempMass));

    roe4Vector += temp4Vector;
  }

  // Add all momenta from neutral ECLClusters
  for (unsigned int iEcl = 0; iEcl < roeClusters.size(); iEcl++) {
    if (roeClusters[iEcl]->isNeutral())
      roe4Vector += roeClusters[iEcl]->get4Vector();
  }

  return roe4Vector;
}

std::map<unsigned int, bool> RestOfEvent::getTrackMask(std::string maskName) const
{
  std::map<unsigned int, bool> emptyMap;

  if (maskName == "")
    return emptyMap;

  if (m_trackMasks.find(maskName) == m_trackMasks.end())
    B2FATAL("Cannot find ROE mask with name \'" << maskName << "\', are you sure you spelled it correctly?");

  return m_trackMasks.at(maskName);
}

std::map<unsigned int, bool> RestOfEvent::getECLClusterMask(std::string maskName) const
{
  std::map<unsigned int, bool> emptyMap;

  if (maskName == "")
    return emptyMap;

  if (m_eclClusterMasks.find(maskName) == m_eclClusterMasks.end())
    B2FATAL("Cannot find ROE mask with name \'" << maskName << "\', are you sure you spelled it correctly?");

  return m_eclClusterMasks.at(maskName);
}

void RestOfEvent::fillFractions(double fractions[], std::string maskName) const
{
  const int n = Const::ChargedStable::c_SetSize;
  double defaultFractions[n];

  // Initialize default as pion always (e, mu, pi, K, prot, deut)
  for (unsigned i = 0; i < n; i++)
    if (i != 2)
      defaultFractions[i] = 0;
    else
      defaultFractions[i] = 1;

  // Initialize fractions array
  for (unsigned i = 0; i < n; i++)
    fractions[i] = 0;

  if (maskName == "")
    for (unsigned i = 0; i < n; i++)
      fractions[i] = defaultFractions[i];

  else if (m_fractionsSet.find(maskName) != m_fractionsSet.end()) {
    std::vector<double> fractionsVector = m_fractionsSet.at(maskName);

    if (fractionsVector.size() == n)
      for (unsigned i = 0; i < n; i++)
        fractions[i] = fractionsVector[i];
    else
      fractions[0] = -1;
  }

  else
    B2FATAL("Cannot find ROE mask with name \'" << maskName << "\', are you sure you spelled it correctly?");
}

int RestOfEvent::getNTracks(std::string maskName) const
{
  int nROETracks = 0;

  if (maskName == "")
    nROETracks = int(m_trackIndices.size());
  else  {
    std::map<unsigned int, bool> trackMask = RestOfEvent::getTrackMask(maskName);
    for (auto const& it : trackMask)
      if (it.second)
        nROETracks++;
  }
  return nROETracks;
}

int RestOfEvent::getNECLClusters(std::string maskName) const
{
  int nROEECLClusters = 0;

  if (maskName == "")
    nROEECLClusters =  int(m_eclClusterIndices.size());
  else  {
    std::map<unsigned int, bool> eclClusterMask = RestOfEvent::getECLClusterMask(maskName);
    for (auto const& it : eclClusterMask)
      if (it.second)
        nROEECLClusters++;
  }
  return nROEECLClusters;
}

void RestOfEvent::print() const
{
  B2INFO(" - Tracks[" << m_trackIndices.size() << "] : ");
  printIndices(m_trackIndices);
  B2INFO(" - ECLCluster[" << m_eclClusterIndices.size() << "] : ");
  printIndices(m_eclClusterIndices);
  B2INFO(" - KLMCluster[" << m_klmClusterIndices.size() << "] : ");
  printIndices(m_klmClusterIndices);
}

void RestOfEvent::printIndices(std::set<int> indices) const
{
  if (indices.empty())
    return;

  std::cout << "     -> ";
  for (const int index : indices) {
    std::cout << index << ", ";
  }
  std::cout << std::endl;
}

ClassImp(RestOfEvent)
