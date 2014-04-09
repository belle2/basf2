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

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>

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
  m_eclClusterIndices.insert(cluster->getArrayIndex());
}

void RestOfEvent::addKLMClusters(const std::vector<int>& indices)
{
  addIndices(indices, m_eclClusterIndices);
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

  if (allECLClusters.getEntries() < getNECLClusters())
    B2ERROR("[RestOfEvent::getAllECLClusters] Number of remaining ECL showers in the RestOfEvent > number of all showers in StoreArray<ECLCluster>!");

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
  if (indices.size() == 0)
    return;

  std::cout << "     -> ";
  for (const int index : indices) {
    std::cout << index << ", ";
  }
  std::cout << std::endl;
}

ClassImp(RestOfEvent)
