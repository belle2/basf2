/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/modules/CDCDedxSkimCDST/CDCDedxSkimCDST.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <framework/datastore/SelectSubset.h>

#include <unordered_set>

using namespace Belle2;
using namespace Dedx;

REG_MODULE(CDCDedxSkimCDST)

CDCDedxSkimCDSTModule::CDCDedxSkimCDSTModule() : Module()
{

  setDescription("Extract dE/dx information for calibration development.");
  addParam("particleLists", m_strParticleList, "Vector of ParticleLists to save", std::vector<std::string>());
  addParam("ListsType", isRecList, "Yes for reco particle list else No ", true);
}

CDCDedxSkimCDSTModule::~CDCDedxSkimCDSTModule() { }

void CDCDedxSkimCDSTModule::initialize()
{

  B2INFO("Selecting two good tracks for radiative bhabha candidates...");

  // required input
  m_dedxTracks.isRequired();

  // declare the subset
  StoreArray<CDCDedxTrack> set("CDCDedxTracks");
  set.isRequired();
  m_selector.registerSubset(set);
  //  m_selector.inheritAllRelations();

  // build a map to relate input strings to the right particle type
  // std::map<std::string, std::string> pdgMap = {{"pi+", "211"}, {"K+", "321"}, {"mu+", "13"}, {"e+", "11"}, {"p+", "2212"}, {"deuteron", "1000010020"}};

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

    //loop over the particles in the list and follow the links to the
    //dE/dx information (Particle -> PIDLikelihood -> Track -> CDCDedxTrack)
    //Gen1 used if supplied list is from FS particle and +Gen2 is used when supply reco list
    for (unsigned int iPGen1 = 0; iPGen1 < particlelist->getListSize(true); iPGen1++) {

      Particle* fPartGen1 = particlelist->getParticle(iPGen1, true);
      if (!fPartGen1) {
        B2WARNING("No particles in the list...");
        continue;
      }

      if (isRecList) {

        for (unsigned iPGen2 = 0; iPGen2 < fPartGen1->getNDaughters(); ++iPGen2) {

          const Particle* fPartGen2 = fPartGen1->getDaughter(iPGen2);

          PIDLikelihood* pid = fPartGen2->getRelatedTo<PIDLikelihood>();
          if (!pid) {
            B2WARNING("Searching w/ 2nd Gen >> No related PID likelihood found");
            continue;
          }

          Track* track = pid->getRelatedFrom<Track>();
          if (!track) {
            B2WARNING("Searching w/ 2nd Gen >> No related Track found");
            continue;
          }

          CDCDedxTrack* dedxTrack = track->getRelatedTo<CDCDedxTrack>();
          if (!dedxTrack) {
            B2WARNING("Searching w/ 2nd Gen >> No related CDCDedxTrack found");
            continue;
          } else {
            indicesToKeep.insert(dedxTrack->getArrayIndex());
          }
        }

      } else {

        PIDLikelihood* pid = fPartGen1->getRelatedTo<PIDLikelihood>();
        if (!pid) {
          B2WARNING("No related PID likelihood found");
          continue;
        }

        Track* track = pid->getRelatedFrom<Track>();
        if (!track) {
          B2WARNING("No related Track found");
          continue;
        }

        CDCDedxTrack* dedxTrack = track->getRelatedTo<CDCDedxTrack>();
        if (!dedxTrack) {
          B2WARNING("No related CDCDedxTrack found");
          continue;
        } else {
          indicesToKeep.insert(dedxTrack->getArrayIndex());
        }

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
{
  B2INFO("CDCDedxSkimCDST terminated: \n");
}
