
/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/modules/CDCDedxHadronCollector/CDCDedxHadronCollectorModule.h>

using namespace Belle2;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCDedxHadronCollector);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
CDCDedxHadronCollectorModule::CDCDedxHadronCollectorModule() : CalibrationCollectorModule()
{
  // Set module properties
  setDescription("A collector module for CDC dE/dx hadron calibrations");

  // Parameter definitions
  addParam("particleLists", m_strParticleList, "Vector of ParticleLists to save", std::vector<std::string>());
  addParam("maxcut", m_maxCut, "high cut on dedx curve for proton sample ", double(1.2));
  addParam("mincut", m_minCut, "low cut on dedx curve for proton sample ", double(0.35));
}

//-----------------------------------------------------------------
//                 Create ROOT objects
//-----------------------------------------------------------------

void CDCDedxHadronCollectorModule::prepare()
{

  std::vector<TTree* > ttree;

  // build a map to relate input strings to the right particle type
  m_pdgMap = {{"pi+", "pion"}, {"K+", "kaon"}, {"mu+", "muon"}, {"e+", "electron"}, {"p+", "proton"}, {"deuteron", "deutron"}};

  for (auto& x : m_pdgMap) {

    // strip the name of the particle lists to make this work
    std::string pdg = x.second;

    ttree.push_back(new TTree(Form("%s", pdg.data()), Form("%s dE/dx information", pdg.data())));
    int tt = ttree.size() - 1;

    ttree[tt]->Branch<double>("dedxnosat", &m_dedxnosat);
    ttree[tt]->Branch<double>("dedx", &m_dedx);
    ttree[tt]->Branch<double>("costh", &m_costh);
    ttree[tt]->Branch<double>("p", &m_p);
    ttree[tt]->Branch<int>("charge", &m_charge);
    ttree[tt]->Branch<int>("nhits", &m_nhits);
    ttree[tt]->Branch<double>("timereso", &m_timeReso);
    ttree[tt]->Branch<double>("injtime", &m_injTime);
    ttree[tt]->Branch<double>("injring", &m_injRing);

    // Collector object registration
    registerObject<TTree>(Form("%s", pdg.data()), ttree[tt]);
  }
}

//-----------------------------------------------------------------
//                 Fill ROOT objects
//-----------------------------------------------------------------
void CDCDedxHadronCollectorModule::collect()
{

  int nParticleList = m_strParticleList.size();

  for (int iList = 0; iList < nParticleList; iList++) {

    //Collector object access
    std::string pdg = m_pdgMap[m_strParticleList[iList].substr(0, m_strParticleList[iList].find(":"))];
    auto tree = getObjectPtr<TTree>(Form("%s", pdg.data()));

    // make sure the list exists and is not empty
    StoreObjPtr<ParticleList> particlelist(m_strParticleList[iList]);

    if (!particlelist or particlelist->getListSize() == 0) continue;

    // loop over the particles in the list and follow the links to the
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
      }

      std::string ptype = m_strParticleList[iList].substr(0, m_strParticleList[iList].find(":"));

      const TrackFitResult* fitResult = track->getTrackFitResult(Const::pion);
      if (ptype != "pi+") {
        if (ptype == "K+") fitResult = track->getTrackFitResultWithClosestMass(Const::kaon);
        else if (ptype == "p+") fitResult = track->getTrackFitResultWithClosestMass(Const::proton);
        else if (ptype == "deuteron") fitResult = track->getTrackFitResultWithClosestMass(Const::deuteron);
        else if (ptype == "mu+") fitResult = track->getTrackFitResultWithClosestMass(Const::muon);
        else if (ptype == "e+") fitResult = track->getTrackFitResultWithClosestMass(Const::electron);
      }

      if (!fitResult) {
        B2WARNING("No related fit for this track...");
        continue;
      }

      if (dedxTrack->size() == 0 || dedxTrack->size() > 200) continue;
      //if out CDC (dont add as we dont correct via correctionmodules)
      if (dedxTrack->getCosTheta() < TMath::Cos(150.0 * TMath::DegToRad()))continue; //-0.866
      if (dedxTrack->getCosTheta() > TMath::Cos(17.0 * TMath::DegToRad())) continue; //0.95

      m_dedxnosat = dedxTrack->getDedxNoSat();
      m_dedx = dedxTrack->getDedx();
      m_p = dedxTrack->getMomentum();
      m_costh = dedxTrack->getCosTheta();
      m_charge = fitResult->getChargeSign();

      if (m_dedx < 0) continue;

      if (ptype == "mu+")
      { if (m_p < 0.05) continue;}

      if (ptype == "p+") {
        if (((m_dedx - 0.45) * abs(m_p) * abs(m_p) > m_maxCut) || ((m_dedx - 0.45)*abs(m_p) * abs(m_p) < m_minCut)) continue;
        if (m_dedx < 1.00) continue;
        if (abs(m_p) > 1.0) continue;
      }

      if (ptype == "pi+") {
        if (m_dedx > 20) continue;
      }

      m_injRing = dedxTrack->getInjectionRing();
      m_injTime = dedxTrack->getInjectionTime();

      m_timeReso = m_DBInjectTime->getCorrection("reso", m_injRing, m_injTime);
      m_nhits = dedxTrack->getNLayerHitsUsed();

      // fill the TTree
      tree->Fill();
    }
  }
}
