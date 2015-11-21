/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
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

void RestOfEvent::addTracks(const std::vector<int>& indices)
{
  addIndices(indices, m_trackIndices);
}

void RestOfEvent::addECLCluster(const ECLCluster* shower)
{
  m_eclClusterIndices.insert(shower->getArrayIndex());
}

void RestOfEvent::addECLClusters(const std::vector<int>& indices)
{
  addIndices(indices, m_eclClusterIndices);
}

void RestOfEvent::addKLMCluster(const KLMCluster* cluster)
{
  m_klmClusterIndices.insert(cluster->getArrayIndex());
}

void RestOfEvent::addKLMClusters(const std::vector<int>& indices)
{
  addIndices(indices, m_klmClusterIndices);
}

void RestOfEvent::setChargedStableFractions(const std::vector<double>& fractions)
{
  const unsigned int n = Const::ChargedStable::c_SetSize;

  if (fractions.size() == n) {
    for (unsigned int i = 0; i < n; i++)
      m_fractions[i] = fractions[i];
  } else if (fractions.size() == 1 && fractions[0] == -1)
    m_useTrueMassHypothesis = true;
  else {
    B2WARNING("Size of fractions vector not appropriate! Default value will be used.");
  }
}

void RestOfEvent::setTrackMasks(std::map<int, bool> masks)
{
  m_trackMasks = masks;
}

void RestOfEvent::setECLClusterMasks(std::map<int, bool> masks)
{
  m_eclClusterMasks = masks;
}

const std::vector<Belle2::Track*> RestOfEvent::getTracks() const
{
  std::vector<Track*> remainTracks(getNTracks());
  StoreArray<Track> allTracks;

  if (allTracks.getEntries() < getNTracks())
    B2ERROR("[RestOfEvent::getAllTracks] Number of remaining tracks in the RestOfEvent > number of all tracks in StoreArray<Track>!");

  int i = 0;
  for (const int index : m_trackIndices) {
    remainTracks[i] = allTracks[index];
    i++;
  }

  return remainTracks;
}

const std::vector<Belle2::ECLCluster*> RestOfEvent::getECLClusters() const
{
  std::vector<ECLCluster*> remainECLClusters(getNECLClusters());
  StoreArray<ECLCluster> allECLClusters;

  if (allECLClusters.getEntries() < getNECLClusters()) {
    B2ERROR("[RestOfEvent::getAllECLClusters] Number of remaining ECL showers in the RestOfEvent > number of all showers in StoreArray<ECLCluster>!");
  }

  int i = 0;
  for (const int index : m_eclClusterIndices) {
    remainECLClusters[i] = allECLClusters[index];
    i++;
  }

  return remainECLClusters;
}

const std::vector<Belle2::KLMCluster*> RestOfEvent::getKLMClusters() const
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

TLorentzVector RestOfEvent::getROE4Vector() const
{
  std::vector<Track*> roeTracks = RestOfEvent::getTracks();
  std::vector<ECLCluster*> roeClusters = RestOfEvent::getECLClusters();
  TLorentzVector roe4Vector;

  // Add all momentum from tracks
  for (unsigned int iTrack = 0; iTrack < roeTracks.size(); iTrack++) {

    const PIDLikelihood* pid = roeTracks[iTrack]->getRelatedTo<PIDLikelihood>();

    if (!pid) {
      B2ERROR("Track with no PID information!");
      continue;
    }

    if (!m_trackMasks.empty())
      if (!m_trackMasks.at(roeTracks[iTrack]->getArrayIndex()))
        continue;

    int particlePDG;

    if (m_useTrueMassHypothesis) {
      const MCParticle* mcp = roeTracks[iTrack]->getRelatedTo<MCParticle>();

      if (!mcp) {
        B2ERROR("No related MCParticle found! Default will be used.");
        particlePDG = Const::pion.getPDGCode();
      }

      particlePDG = abs(mcp->getPDG());
    } else {
      particlePDG = pid->getMostLikely(m_fractions).getPDGCode();
    }

    Const::ChargedStable trackParticle = Const::ChargedStable(particlePDG);
    const TrackFitResult* tfr = roeTracks[iTrack]->getTrackFitResult(trackParticle);

    // Update energy of tracks (default: pion always)
    float tempMass = trackParticle.getMass();
    TVector3 tempMom = tfr->getMomentum();
    TLorentzVector temp4Vector;
    temp4Vector.SetVect(tempMom);
    temp4Vector.SetE(TMath::Sqrt(tempMom.Mag2() + tempMass * tempMass));

    roe4Vector += temp4Vector;
  }

  // Add all momentum from neutral ECLClusters
  for (unsigned int iEcl = 0; iEcl < roeClusters.size(); iEcl++) {

    if (!roeClusters[iEcl]->isNeutral()) continue;

    if (!m_eclClusterMasks.empty())
      if (!m_eclClusterMasks.at(roeClusters[iEcl]->getArrayIndex()))
        continue;

    roe4Vector += roeClusters[iEcl]->get4Vector();
  }

  return roe4Vector;
}

std::map<int, bool> RestOfEvent::getTrackMasks() const
{
  return m_trackMasks;
}

std::map<int, bool> RestOfEvent::getECLClusterMasks() const
{
  return m_eclClusterMasks;
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
