/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/CDCDedxSkimCDST/CDCDedxSkimCDST.h>
#include <framework/datastore/SelectSubset.h>

#include <unordered_set>

using namespace Belle2;
using namespace Dedx;

REG_MODULE(CDCDedxSkimCDST)

CDCDedxSkimCDSTModule::CDCDedxSkimCDSTModule() : Module()
{

  setDescription("Extract dE/dx information for calibration development.");

  addParam("particleLists", m_strParticleList, "Vector of ParticleLists to save", std::vector<std::string>());
}

CDCDedxSkimCDSTModule::~CDCDedxSkimCDSTModule() { }

void CDCDedxSkimCDSTModule::initialize()
{

  B2INFO("Creating a ROOT file for the hit level information...");

  // required input
  m_dedxTracks.isRequired();

  // declare the subset
  StoreArray<CDCDedxTrack> set("CDCDedxTracks");
  set.isRequired();
  m_selector.registerSubset(set);
  //  m_selector.inheritAllRelations();

  // build a map to relate input strings to the right particle type
  std::map<std::string, std::string> pdgMap = {{"pi+", "211"}, {"K+", "321"}, {"mu+", "13"}, {"e+", "11"}, {"p+", "2212"}, {"deuteron", "1000010020"}};
}

void CDCDedxSkimCDSTModule::event()
{

  int nParticleList = m_strParticleList.size();
  std::unordered_set<int> indicesToKeep;

  // **************************************************
  //
  //  LOOP OVER particles in the given particle lists
  //
  // **************************************************

  for (int iList = 0; iList < nParticleList; iList++) {
    // make sure the list exists and is not empty
    StoreObjPtr<ParticleList> particlelist(m_strParticleList[iList]);
    if (!particlelist or particlelist->getListSize(true) == 0) {
      continue;
    }

    // loop over the particles in the list and follow the links to the
    // dE/dx information (Particle -> PIDLikelihood -> Track -> CDCDedxTrack)
    for (unsigned int iPart = 0; iPart < particlelist->getListSize(true); iPart++) {
      Particle* part = particlelist->getParticle(iPart, true);
      if (!part) {
        B2WARNING("No particles...");
        continue;
      }
      PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid) {
        B2WARNING("No related PID likelihood...");
        continue;
      }
      Track* track = pid->getRelatedFrom<Track>();
      if (!track) {
        B2WARNING("No related track...");
        continue;
      }
      CDCDedxTrack* dedxTrack = track->getRelatedTo<CDCDedxTrack>();
      if (!dedxTrack) {
        B2WARNING("No related CDCDedxTrack...");
        continue;
      } else {
        indicesToKeep.insert(dedxTrack->getArrayIndex());
      }
    }
  }

  // remove tracks with no Particle association
  auto selector = [indicesToKeep](const CDCDedxTrack * d) -> bool {
    if (indicesToKeep.size() > 2)
    {
      std::cout << "More than three good tracks!" << std::endl;
      return false;
    }
    int idx = d->getArrayIndex();
    if (indicesToKeep.count(idx) == 1) return true;
    return false;
  };
  m_selector.select(selector);
}

void CDCDedxSkimCDSTModule::terminate()
{}
