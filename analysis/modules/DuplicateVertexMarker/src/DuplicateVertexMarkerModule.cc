/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
//Identify duplicate vertices (distinct particles, but built from the same daughters).
//Only works if the particle has exactly two daughters. Mainly used to deal when merging V0 vertices with hand-built ones.

//Functionality is designed to be expanded as needed.

#include <analysis/modules/DuplicateVertexMarker/DuplicateVertexMarkerModule.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/VariableManager/Variables.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>

#include <set>

using namespace std;
using namespace Belle2;


//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(DuplicateVertexMarker)

DuplicateVertexMarkerModule::DuplicateVertexMarkerModule() : Module(), m_targetVar(nullptr)
{
  setDescription("Identify duplicate vertices (distinct particles, but built from the same daughters) and mark the one with best chi2. Only works if the particle has exactly two daughters. Mainly used to deal when merging V0 vertices with hand-built ones.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Add parameters
  addParam("particleList", m_particleList, "Input ParticleList name");
  addParam("extraInfoName", m_extraInfoName,
           "Extra-info field added to all particles in the input list. 1 for the best vertex, 0 for lower ranked ones.",
           string("highQualityVertex"));
  addParam("prioritiseV0", m_prioritiseV0,
           "If a vertex is a V0, select it over its duplicate even if chi2 is worse.",
           true);
}

void DuplicateVertexMarkerModule::initialize()
{
  StoreObjPtr<ParticleList>().isRequired(m_particleList);

  Variable::Manager& manager = Variable::Manager::Instance();
  m_targetVar = manager.getVariable("chiProb");
  if (m_targetVar == nullptr) {
    B2ERROR("DuplicateVertexMarker: Variable::Manager doesn't have variable chiProb");
  }
}

void DuplicateVertexMarkerModule::event()
{
  const StoreObjPtr<ParticleList> inPList(m_particleList);
  if (!inPList)
    return;

  const int size = inPList->getListSize();
  std::set<const Particle*> foundParticles;
  for (int i = 0; i < size; i++) {
    Particle* part = inPList->getParticle(i);
    if (part->getNDaughters() != 2) { //ignore 3+ vertices
      B2WARNING("Vertex does not have exactly 2 daughters! SKIP.");
      continue;
    }
    if (part->hasExtraInfo(
          m_extraInfoName)) { //if it already has info, it means it's already been discarded (or it won, but that shouldn't be possible here)
      B2DEBUG(10, "Extra Info with given name is already set!");
      continue;
    }
    for (int j = 0; j < size; j++) {//look for a clone among other particles in the event
      Particle* cloneCand = inPList->getParticle(j);
      bool particleFight = false;
      if (cloneCand->getNDaughters() == 2) { //check if it's another 2-vertex with the same exact daughters
        if (part == cloneCand) continue; //but not itself
        if (cloneCand->hasExtraInfo(m_extraInfoName)) continue; //nor an already discarded one
        B2DEBUG(10, "part has daughters (" << part->getDaughter(0)->getTrack() << ") and (" << part->getDaughter(1)->getTrack() << ")");
        B2DEBUG(10, "cloneCand has daughters (" << cloneCand->getDaughter(0)->getTrack() << ") and (" << cloneCand->getDaughter(
                  1)->getTrack() << ")");
        if (part->getDaughter(0)->getTrack() == cloneCand->getDaughter(0)->getTrack() &&
            part->getDaughter(1)->getTrack() == cloneCand->getDaughter(1)->getTrack()) {
          particleFight = true;
        } else if (part->getDaughter(0)->getTrack() == cloneCand->getDaughter(1)->getTrack() &&
                   part->getDaughter(1)->getTrack() == cloneCand->getDaughter(0)->getTrack()) {
          particleFight = true;
        }
      }
      if (particleFight) { //fight; whoever loses gets a low quality flag
        if (m_prioritiseV0) { //V0 have no dmID info (might not be the optimal way to tell them apart)
          bool partNotV0 = part->hasExtraInfo("decayModeID");
          bool cloneNotV0 = cloneCand->hasExtraInfo("decayModeID");
          if (partNotV0 != cloneNotV0) { //one of them is V0 and the other not
            (partNotV0) ? (part->addExtraInfo(m_extraInfoName, 0.0)) : (cloneCand->addExtraInfo(m_extraInfoName, 0.0));
            if (partNotV0) {
              B2DEBUG(10, "V0: Discarding Particle.");
            } else B2DEBUG(10, "V0: Discarding Clone");
          }
        }
        if (!(part->hasExtraInfo(m_extraInfoName) || cloneCand->hasExtraInfo(m_extraInfoName))) {
          //if V0s aren't being checked, or have been checked but inconclusive (should not happen) check best fit
          B2DEBUG(10, m_targetVar->function(part) << " vs " << m_targetVar->function(cloneCand));
          if (m_targetVar->function(part) > m_targetVar->function(cloneCand)) {
            cloneCand->addExtraInfo(m_extraInfoName, 0.0);
          } else {
            part->addExtraInfo(m_extraInfoName, 0.0);
            continue; //The particle lost, and its clone will be picked up next in the for(i) loop.
          }
        }
      }
    }
    if (!(part->hasExtraInfo(m_extraInfoName))) { //if it got there without a flag, there are no rival particles anymore;
      part->addExtraInfo(m_extraInfoName, 1.0);//this is the best
      continue; //no point in continuing the i-loop
    }
  }
}
