/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/SensitiveDetectorBase.h>
#include <simulation/kernel/RunManager.h>
#include <simulation/kernel/UserInfo.h>
#include <framework/logging/Logger.h>
#include <generators/dataobjects/MCParticleGraph.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;


SensitiveDetectorBase::SensitiveDetectorBase(G4String name) : G4VSensitiveDetector(name)
{
  m_hitMCParticleColName = "";
}


SensitiveDetectorBase::~SensitiveDetectorBase()
{

}


void SensitiveDetectorBase::addRelationCollection(const std::string& hitMCParticleColName)
{
  m_hitMCParticleColName = hitMCParticleColName;
}


void SensitiveDetectorBase::setSeenInDetectorFlag(G4Step* step, MCParticle::StatusBit subdetectorBit)
{
  TrackInfo::getInfo(*step->GetTrack()).addStatus(subdetectorBit);
}
