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

#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLGamma.h>
#include <ecl/dataobjects/ECLPi0.h>
#include <tracking/dataobjects/Track.h>

#include <framework/logging/Logger.h>
#include <iostream>

using namespace Belle2;

void RestOfEvent::addTrack(const Track* track)
{
  m_trackIndices.insert(track->getArrayIndex());
}

void RestOfEvent::addECLShower(const ECLShower* shower)
{
  m_eclShowerIndices.insert(shower->getArrayIndex());
}

void RestOfEvent::addECLPi0(const ECLPi0* pi0)
{
  m_eclPi0Indices.insert(pi0->getArrayIndex());
}

void RestOfEvent::addECLGamma(const ECLGamma* gamma)
{
  m_eclGammaIndices.insert(gamma->getArrayIndex());
}

void RestOfEvent::addTracks(const std::vector<int> indices)
{
  addIndices(indices, m_trackIndices);
}

void RestOfEvent::addECLShowers(const std::vector<int> indices)
{
  addIndices(indices, m_eclShowerIndices);
}

void RestOfEvent::addECLGammas(const std::vector<int> indices)
{
  addIndices(indices, m_eclGammaIndices);
}

void RestOfEvent::addECLPi0s(const std::vector<int> indices)
{
  addIndices(indices, m_eclPi0Indices);
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

const std::vector<Belle2::ECLShower*> RestOfEvent::getECLShowers() const
{
  std::vector<ECLShower*> remainECLShowers(getNECLShowers());
  StoreArray<ECLShower> allECLShowers;

  if (allECLShowers.getEntries() < getNECLShowers())
    B2ERROR("[RestOfEvent::getAllECLShowers] Number of remaining ECL showers in the RestOfEvent > number of all tracks in StoreArray<ECLShower>!");

  int i = 0;
  for (const int index : m_eclShowerIndices) {
    remainECLShowers[i] = allECLShowers[index];
    i++;
  }

  return remainECLShowers;
}

const std::vector<Belle2::ECLGamma*> RestOfEvent::getECLGammas() const
{
  std::vector<ECLGamma*> remainECLGammas(getNECLGammas());
  StoreArray<ECLGamma> allECLGammas;

  if (allECLGammas.getEntries() < getNECLGammas())
    B2ERROR("[RestOfEvent::getAllECLGammas] Number of remaining ECL gammas in the RestOfEvent > number of all tracks in StoreArray<ECLGamma>!");

  int i = 0;
  for (const int index : m_eclGammaIndices) {
    remainECLGammas[i] = allECLGammas[index];
    i++;
  }

  return remainECLGammas;
}

const std::vector<Belle2::ECLPi0*> RestOfEvent::getECLPi0s() const
{
  std::vector<ECLPi0*> remainECLPi0s(getNECLPi0s());
  StoreArray<ECLPi0> allECLPi0s;

  if (allECLPi0s.getEntries() < getNECLPi0s())
    B2ERROR("[RestOfEvent::getAllECLPi0s] Number of remaining ECL gammas in the RestOfEvent > number of all tracks in StoreArray<ECLPi0>!");

  int i = 0;
  for (const int index : m_eclPi0Indices) {
    remainECLPi0s[i] = allECLPi0s[index];
    i++;
  }

  return remainECLPi0s;
}

void RestOfEvent::print() const
{
  B2INFO(" - Tracks[" << m_trackIndices.size() << "] : ");
  printIndices(m_trackIndices);
  B2INFO(" - ECLShower[" << m_eclShowerIndices.size() << "] : ");
  printIndices(m_eclShowerIndices);
  B2INFO(" - ECLGamma[" << m_eclGammaIndices.size() << "] : ");
  printIndices(m_eclGammaIndices);
  B2INFO(" - ECLPi0[" << m_eclPi0Indices.size() << "] : ");
  printIndices(m_eclPi0Indices);
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
