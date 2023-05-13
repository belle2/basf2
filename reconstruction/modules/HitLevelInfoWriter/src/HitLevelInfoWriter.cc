/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/modules/HitLevelInfoWriter/HitLevelInfoWriter.h>
#include <mdst/dbobjects/BeamSpot.h>
#include <TMath.h>

using namespace Belle2;
using namespace Dedx;
using namespace std;

REG_MODULE(HitLevelInfoWriter);

HitLevelInfoWriterModule::HitLevelInfoWriterModule() : Module()
{

  setDescription("Extract dE/dx information for calibration development.");

  addParam("outputBaseName", m_strOutputBaseName, "Suffix for output file name", string("HLInfo.root"));
  addParam("particleLists", m_strParticleList, "Vector of ParticleLists to save", vector<string>());
  addParam("enableHitLevel", m_isHitLevel, "True or False for Hit level variables", false);
  addParam("m_isExtraVar", m_isExtraVar, "True or False for extra track/hit level variables", false);
  addParam("nodeadwire", m_isDeadwire, "True or False for deadwire hit variables", false);
  addParam("relativeCorrections", m_isRelative, "If true, apply corrections relative to those used in production", false);
  addParam("corrections", m_isCorrection, "If true, apply corrections", true);

}

HitLevelInfoWriterModule::~HitLevelInfoWriterModule() { }

void HitLevelInfoWriterModule::initialize()
{

  B2INFO("Creating a ROOT file for the hit level information...");

  // required inputs
  m_dedxTracks.isRequired();
  m_tracks.isRequired();
  m_trackFitResults.isRequired();
  m_eclClusters.isOptional();
  m_klmClusters.isOptional();

  // build a map to relate input strings to the right particle type
  map<string, string> pdgMap = {{"pi+", "Const::pion.getPDGCode()"}, {"K+", "Const::kaon.getPDGCode()"}, {"mu+", "Const::muon.getPDGCode()"}, {"e+", "Const::electron.getPDGCode()"}, {"p+", "Const::proton.getPDGCode()"}, {"deuteron", "Const::deuteron.getPDGCode()"}};

  // if no particle lists are given, write out all tracks
  if (m_strParticleList.size() == 0) bookOutput(m_strOutputBaseName);

  // create a new output file for each particle list specified
  for (unsigned int i = 0; i < m_strParticleList.size(); i++) {
    // strip the name of the particle lists to make this work
    string pdg = pdgMap[m_strParticleList[i].substr(0, m_strParticleList[i].find(":"))];
    string filename = string(m_strOutputBaseName + "_PID" + pdg + ".root");
    bookOutput(filename);
  }
}

void HitLevelInfoWriterModule::event()
{

  int nParticleList = m_strParticleList.size();

  // **************************************************
  //  LOOP OVER dE/dx measurements for all tracks if
  //    no particle list is specified.
  // **************************************************
  if (nParticleList == 0) {
    for (int idedx = 0; idedx < m_dedxTracks.getEntries(); idedx++) {

      CDCDedxTrack* dedxTrack = m_dedxTracks[idedx];
      if (!dedxTrack) {
        B2WARNING("No dedx related track...");
        continue;
      }

      const Track* track = dedxTrack->getRelatedFrom<Track>();
      if (!track) {
        B2WARNING("No related track...");
        continue;
      }

      const TrackFitResult* fitResult = track->getTrackFitResultWithClosestMass(Const::pion);
      if (!fitResult) {
        B2WARNING("No related fit for this track...");
        continue;
      }

      if (dedxTrack->size() == 0 || dedxTrack->size() > 200) continue;
      if (dedxTrack->getCosTheta() < TMath::Cos(150.0 * TMath::DegToRad()))continue; //-0.866
      if (dedxTrack->getCosTheta() > TMath::Cos(17.0 * TMath::DegToRad())) continue; //0.95

      // fill the event meta data
      StoreObjPtr<EventMetaData> evtMetaData;
      m_expID = evtMetaData->getExperiment();
      m_runID = evtMetaData->getRun();
      m_eventID = evtMetaData->getEvent();

      m_injring = dedxTrack->getInjectionRing();
      m_injtime = dedxTrack->getInjectionTime();

      //--------REMOVEBAL--------
      //when CDST are reproduced with injection time
      if (m_injtime == -1 || m_injring == -1) {
        if (m_TTDInfo.isValid() && m_TTDInfo->hasInjection()) {
          m_injring = m_TTDInfo->isHER();
          m_injtime =  m_TTDInfo->getTimeSinceLastInjectionInMicroSeconds();
        }
      }
      //--------REMOVEBAL--------

      // fill the E/P
      const ECLCluster* eclCluster = track->getRelated<ECLCluster>();
      if (eclCluster and eclCluster->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) {
        m_eop = (eclCluster->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons)) / (fitResult->getMomentum().R());
        m_e = eclCluster->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons);
        if (m_isExtraVar) {
          m_e1_9 = eclCluster->getE1oE9();
          m_e9_21 = eclCluster->getE9oE21();
          m_eclsnHits = eclCluster->getNumberOfCrystals();
        }
        // fill the muon depth
        const KLMCluster* klmCluster = eclCluster->getRelated<KLMCluster>();
        if (klmCluster) m_klmLayers = klmCluster->getLayers();
      }

      // fill the TTree with the Track information
      fillTrack(fitResult);

      // fill the TTree with the CDCDedxTrack information
      fillDedx(dedxTrack);

      // fill the TTree
      m_tree[0]->Fill();
    }
  }

  // **************************************************
  //
  //  LOOP OVER particles in the given particle lists
  //
  // **************************************************
  for (int iList = 0; iList < nParticleList; iList++) {
    // make sure the list exists and is not empty
    StoreObjPtr<ParticleList> particlelist(m_strParticleList[iList]);
    if (!particlelist or particlelist->getListSize(true) == 0) {
      //B2WARNING("ParticleList " << m_strParticleList[iList] << " not found or empty, skipping");
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
      }
      string ptype = m_strParticleList[iList].substr(0, m_strParticleList[iList].find(":"));
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

      // fill the event meta data
      StoreObjPtr<EventMetaData> evtMetaData;
      m_expID = evtMetaData->getExperiment();
      m_runID = evtMetaData->getRun();
      m_eventID = evtMetaData->getEvent();

      m_injring = dedxTrack->getInjectionRing();
      m_injtime = dedxTrack->getInjectionTime();

      //--------REMOVEBAL--------
      //when CDST are reproduced with injection time
      if (m_injtime == -1 || m_injring == -1) {
        if (m_TTDInfo.isValid() && m_TTDInfo->hasInjection()) {
          m_injring = m_TTDInfo->isHER();
          m_injtime =  m_TTDInfo->getTimeSinceLastInjectionInMicroSeconds();
        }
      }
      //--------REMOVEBAL--------

      // fill the E/P
      const ECLCluster* eclCluster = track->getRelated<ECLCluster>();
      if (eclCluster and eclCluster->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) {
        m_eop = (eclCluster->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons)) / (fitResult->getMomentum().R());
        m_e = eclCluster->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons);
        if (m_isExtraVar) {
          m_e1_9 = eclCluster->getE1oE9();
          m_e9_21 = eclCluster->getE9oE21();
          m_eclsnHits = eclCluster->getNumberOfCrystals();
        }
        // fill the muon depth
        const KLMCluster* klmCluster = eclCluster->getRelated<KLMCluster>();
        if (klmCluster) m_klmLayers = klmCluster->getLayers();
      }

      // fill the TTree with the Track information
      fillTrack(fitResult);

      // fill the TTree with the CDCDedxTrack information
      fillDedx(dedxTrack);

      // fill the TTree
      m_tree[iList]->Fill();
    }
  }
}

//---------------------------------------------------------------------------------------
void HitLevelInfoWriterModule::terminate()
{

  for (unsigned int i = 0; i < m_file.size(); i++) {
    B2INFO("Done writing out the hit level information...\t" << m_tree[i]->GetEntries() << " tracks");

    // write the ttree to a root file
    m_file[i]->cd();
    m_tree[i]->Write();
    m_file[i]->Close();
  }
}

//---------------------------------------------------------------------------------------
void HitLevelInfoWriterModule::fillTrack(const TrackFitResult* fitResult)
{
  ROOT::Math::XYZVector trackMom = fitResult->getMomentum();
  m_p = trackMom.R();
  m_pt = trackMom.Rho();
  m_phi = trackMom.Phi();

  m_theta = trackMom.Theta() * 180. / TMath::Pi(); //in degree
  if (m_theta > 17. && m_theta < 150.)m_inCDC =  1;
  else m_inCDC = 0;

  if (fitResult->getChargeSign() < 0) {
    m_p *= -1;
    m_pt *= -1;
  }

  ROOT::Math::XYZVector trackPos = fitResult->getPosition();
  m_vx0 = trackPos.X();
  m_vy0 = trackPos.Y();
  m_vz0 = trackPos.Z();

  m_d0 = fitResult->getD0();
  m_z0 = fitResult->getZ0();
  m_chi2 = fitResult->getPValue();
  m_tanlambda = fitResult->getTanLambda();
  m_phi0 = fitResult->getPhi0();
  m_nCDChits =  fitResult->getHitPatternCDC().getNHits();

  static DBObjPtr<BeamSpot> beamSpotDB;
  const auto& frame = ReferenceFrame::GetCurrent();
  UncertainHelix helix = fitResult->getUncertainHelix();
  helix.passiveMoveBy(ROOT::Math::XYZVector(beamSpotDB->getIPPosition()));
  m_dr = frame.getVertex(ROOT::Math::XYZVector(helix.getPerigee())).Rho();
  m_dphi = frame.getVertex(ROOT::Math::XYZVector(helix.getPerigee())).Phi();
  m_dz = frame.getVertex(ROOT::Math::XYZVector(helix.getPerigee())).Z();

}

//---------------------------------------------------------------------------------------
void HitLevelInfoWriterModule::fillDedx(CDCDedxTrack* dedxTrack)
{
  // clear the containers first
  clearEntries();

  m_trackID = dedxTrack->trackID();
  m_length = dedxTrack->getLength();
  m_charge = dedxTrack->getCharge();
  m_cosTheta = dedxTrack->getCosTheta();
  m_PDG = dedxTrack->getPDG();

  m_pCDC = dedxTrack->getMomentum();
  if (m_charge < 0) {
    m_pCDC *= -1;
  }

  h_nhits = dedxTrack->size();

  // Get the calibration constants
  m_scale = m_DBScaleFactor->getScaleFactor();
  m_runGain = m_DBRunGain->getRunGain();
  m_cosCor = m_DBCosineCor->getMean(m_cosTheta);
  m_timeGain = m_DBInjectTime->getCorrection("mean", m_injring, m_injtime);
  m_timeReso = m_DBInjectTime->getCorrection("reso", m_injring, m_injtime);

  if (m_cosTheta <= -0.850 || m_cosTheta >= 0.950) {
    m_cosEdgeCor = m_DBCosEdgeCor->getMean(m_cosTheta);
  } else {
    m_cosEdgeCor = 1.0;
  }
  m_hadronpars = m_DBHadronCor->getHadronPars();

  //variable to save layer variables
  map<int, vector<double>>l_var;
  double cdcChi[Const::ChargedStable::c_SetSize];

  //Modify the hit level dedx
  if (m_isCorrection) {
    recalculateDedx(dedxTrack, l_var, cdcChi);
    m_chie = cdcChi[0];
    m_chimu = cdcChi[1];
    m_chipi = cdcChi[2];
    m_chik = cdcChi[3];
    m_chip = cdcChi[4];
    m_chid = cdcChi[5];
  } else {
    m_chie = dedxTrack->getChi(0);
    m_chimu = dedxTrack->getChi(1);
    m_chipi = dedxTrack->getChi(2);
    m_chik = dedxTrack->getChi(3);
    m_chip = dedxTrack->getChi(4);
    m_chid = dedxTrack->getChi(5);
  }

  m_pmeane = dedxTrack->getPmean(0);
  m_pmeanmu = dedxTrack->getPmean(1);
  m_pmeanpi = dedxTrack->getPmean(2);
  m_pmeank = dedxTrack->getPmean(3);
  m_pmeanp = dedxTrack->getPmean(4);
  m_pmeand = dedxTrack->getPmean(5);

  m_prese = dedxTrack->getPreso(0);
  m_presmu = dedxTrack->getPreso(1);
  m_prespi = dedxTrack->getPreso(2);
  m_presk = dedxTrack->getPreso(3);
  m_presp = dedxTrack->getPreso(4);
  m_presd = dedxTrack->getPreso(5);

  // Get the vector of dE/dx values for all layers
  double lout = 0, lin = 0, increment = 0;
  int lastlayer = 0;

  if (m_isCorrection) {
    l_nhits = l_var[0].size();
    const int lEdgeTrunc = int(l_nhits * 0.05 + 0.51);
    const int hEdgeTrunc = int(l_nhits * (1 - 0.25) + 0.51);
    l_nhitsused = hEdgeTrunc - lEdgeTrunc;
  } else {
    l_nhits = dedxTrack->getNLayerHits();
    l_nhitsused = dedxTrack->getNLayerHitsUsed();
  }

  for (int il = 0; il < l_nhits; ++il) {
    if (m_isCorrection) {
      l_nhitscombined[il] = l_var[0][il];
      l_wirelongesthit[il] = l_var[1][il];
      l_layer[il] = l_var[2][il];
      l_path[il] = l_var[3][il];
      l_dedx[il] = l_var[4][il];
    } else {
      l_nhitscombined[il] = dedxTrack->getNHitsCombined(il);
      l_wirelongesthit[il] = dedxTrack->getWireLongestHit(il);
      l_layer[il] = dedxTrack->getLayer(il);
      l_path[il] = dedxTrack->getLayerPath(il);
      l_dedx[il] = dedxTrack->getLayerDedx(il);
    }
    if (l_layer[il] > lastlayer) lout++;
    else if (l_layer[il] < lastlayer) lin++;
    else continue;

    lastlayer = l_layer[il];
    increment++;
  }
  m_ioasym = (lout - lin) / increment;

  // Get the vector of dE/dx values for all hits
  if (m_isHitLevel) {
    for (int ihit = 0; ihit < h_nhits; ++ihit) {

      if (m_isDeadwire) continue;

      h_lwire[ihit] = dedxTrack->getWireInLayer(ihit);
      h_wire[ihit] = dedxTrack->getWire(ihit);
      h_layer[ihit] = dedxTrack->getHitLayer(ihit);
      h_path[ihit] = dedxTrack->getPath(ihit);
      h_dedx[ihit] = dedxTrack->getDedx(ihit);
      h_adcraw[ihit] = dedxTrack->getADCBaseCount(ihit);
      h_adccorr[ihit] = dedxTrack->getADCCount(ihit);
      h_doca[ihit] = dedxTrack->getDoca(ihit);
      h_ndoca[ihit] = h_doca[ihit] / dedxTrack->getCellHalfWidth(ihit);
      h_ndocaRS[ihit] = dedxTrack->getDocaRS(ihit) / dedxTrack->getCellHalfWidth(ihit);
      h_enta[ihit] = dedxTrack->getEnta(ihit);
      h_entaRS[ihit] = dedxTrack->getEntaRS(ihit);
      h_driftT[ihit] = dedxTrack->getDriftT(ihit);
      h_driftD[ihit] = dedxTrack->getDriftD(ihit);

      // Get extra variable from tracking
      if (m_isExtraVar) {
        h_WeightPionHypo[ihit] = dedxTrack->getWeightPionHypo(ihit);
        h_WeightKaonHypo[ihit] = dedxTrack->getWeightKaonHypo(ihit);
        h_WeightProtonHypo[ihit] = dedxTrack->getWeightProtonHypo(ihit);
        h_foundByTrackFinder[ihit] = dedxTrack->getFoundByTrackFinder(ihit);
      }
      // Get the calibration constants
      h_facnladc[ihit] = dedxTrack->getNonLADCCorrection(ihit);
      h_wireGain[ihit] = m_DBWireGains->getWireGain(h_wire[ihit]);
      h_twodCor[ihit] = m_DB2DCell->getMean(h_layer[ihit], h_ndocaRS[ihit], h_entaRS[ihit]);
      h_onedCor[ihit] = m_DB1DCell->getMean(h_layer[ihit], h_entaRS[ihit]);
    }
  }

}

//---------------------------------------------------------------------------------------
void HitLevelInfoWriterModule::recalculateDedx(CDCDedxTrack* dedxTrack, map<int, vector<double>>& l_var,
                                               double (&cdcChi)[Const::ChargedStable::c_SetSize])
{
  vector<double> newLayerHits;
  double newLayerDe = 0, newLayerDx = 0;
  int nhitscombined = 0; // number of hits combined per layer
  int wirelongesthit = 0; // wire number of longest hit
  double longesthit = 0; // path length of longest hit

  for (int ihit = 0; ihit < h_nhits; ++ihit) {
    int jadcbase = dedxTrack->getADCBaseCount(ihit);
    int jLayer = dedxTrack->getHitLayer(ihit);
    double jWire = dedxTrack->getWire(ihit);
    double jNDocaRS = dedxTrack->getDocaRS(ihit) / dedxTrack->getCellHalfWidth(ihit);
    double jEntaRS = dedxTrack->getEntaRS(ihit);
    double jPath = dedxTrack->getPath(ihit);

    double correction = dedxTrack->getScaleFactor() * dedxTrack->getRunGain() * dedxTrack->getTimeMean() *
                        dedxTrack->getCosineCorrection() * dedxTrack->getCosEdgeCorrection() *
                        dedxTrack->getTwoDCorrection(ihit) * dedxTrack->getOneDCorrection(ihit) *
                        dedxTrack->getNonLADCCorrection(ihit);
    if (dedxTrack->getWireGain(ihit) > 0) correction *= dedxTrack->getWireGain(ihit); //also keep dead wire

    if (m_isRelative) {
      //get same base adc + rel correction factor
      correction *= GetCorrection(jadcbase, jLayer, jWire, jNDocaRS, jEntaRS, m_cosTheta, m_injring, m_injtime);
      if (!m_DBWireGains && dedxTrack->getWireGain(ihit) == 0) correction = 0;
    } else {
      //get modifed adc + abs correction factor
      correction = GetCorrection(jadcbase, jLayer, jWire, jNDocaRS, jEntaRS, m_cosTheta, m_injring, m_injtime);
    }

    double newhitdedx = 1.0;

    newhitdedx *= jadcbase * sqrt(1 - m_cosTheta * m_cosTheta) / jPath;

    if (correction != 0) {
      newhitdedx /= correction;
      if (m_DBWireGains->getWireGain(jWire) != 0) {
        newLayerDe += jadcbase / correction;
        newLayerDx += jPath;
        if (jPath > longesthit) {
          longesthit = jPath;
          wirelongesthit = jWire;
        }
        nhitscombined++;
      }
    } else newhitdedx = 0;

    dedxTrack->setDedx(ihit, newhitdedx);

    if (ihit + 1 < h_nhits && dedxTrack->getHitLayer(ihit + 1) == jLayer) {
      continue;
    } else {
      if (newLayerDx != 0) {
        double totalDistance = newLayerDx / sqrt(1 - m_cosTheta * m_cosTheta);
        double newLayerDedx = newLayerDe / totalDistance ;
        newLayerHits.push_back(newLayerDedx);
        l_var[0].push_back(nhitscombined);
        l_var[1].push_back(wirelongesthit);
        l_var[2].push_back(jLayer);
        l_var[3].push_back(totalDistance);
        l_var[4].push_back(newLayerDedx);
      }

      newLayerDe = 0;
      newLayerDx = 0;
      nhitscombined = 0;
      wirelongesthit = 0;
      longesthit = 0;
    }
  }

  // recalculate the truncated means
  double dedxmean, dedxtrunc, dedxtruncNoSat, dedxerror;

  calculateMeans(&dedxmean, &dedxtruncNoSat, &dedxerror, newLayerHits);

  dedxtrunc = dedxtruncNoSat;

  HadronCorrection(m_cosTheta, dedxtrunc);
  m_mean = dedxmean;
  m_trunc = dedxtrunc;
  m_truncNoSat = dedxtruncNoSat;
  m_error = dedxerror;

  // save the PID information
  saveChiValue(cdcChi, dedxTrack, m_trunc);
}

//---------------------------------------------------------------------------------------
double HitLevelInfoWriterModule::GetCorrection(int& adc, int layer, int wireID, double doca, double enta,
                                               double costheta, double ring, double time) const
{
  double correction = 1.0;
  correction *= m_DBScaleFactor->getScaleFactor();
  correction *= m_DBRunGain->getRunGain();
  correction *= m_DB2DCell->getMean(layer, doca, enta);
  correction *= m_DB1DCell->getMean(layer, enta);
  correction *= m_DBCosineCor->getMean(costheta);
  correction *= m_DBInjectTime->getCorrection("mean", ring, time);
  if (costheta <= -0.850 || costheta >= 0.950) correction *= m_DBCosEdgeCor->getMean(costheta);
  if (m_DBWireGains->getWireGain(wireID) > 0)  correction *= m_DBWireGains->getWireGain(wireID);

  //last is only for abs constant
  if (!m_isRelative) adc = m_DBNonlADC->getCorrectedADC(adc, layer);

  return correction;
}

//---------------------------------------------------------------------------------------
void HitLevelInfoWriterModule::HadronCorrection(double costheta, double& dedx) const
{
  dedx = D2I(costheta, I2D(costheta, 1.00) / 1.00 * dedx);
}

//---------------------------------------------------------------------------------------
double HitLevelInfoWriterModule::D2I(const double cosTheta, const double D) const
{
  double absCosTheta   = fabs(cosTheta);
  double projection    = pow(absCosTheta, m_hadronpars[3]) + m_hadronpars[2];
  if (projection == 0) {
    B2WARNING("Something wrong with dE/dx hadron constants!");
    return D;
  }

  double chargeDensity = D / projection;
  double numerator     = 1 + m_hadronpars[0] * chargeDensity;
  double denominator   = 1 + m_hadronpars[1] * chargeDensity;

  if (denominator == 0) {
    B2WARNING("Something wrong with dE/dx hadron constants!");
    return D;
  }

  double I = D * m_hadronpars[4] * numerator / denominator;
  return I;
}

//---------------------------------------------------------------------------------------
double HitLevelInfoWriterModule::I2D(const double cosTheta, const double I) const
{
  double absCosTheta = fabs(cosTheta);
  double projection  = pow(absCosTheta, m_hadronpars[3]) + m_hadronpars[2];

  if (projection == 0 || m_hadronpars[4] == 0) {
    B2WARNING("Something wrong with dE/dx hadron constants!");
    return I;
  }

  double a =  m_hadronpars[0] / projection;
  double b =  1 - m_hadronpars[1] / projection * (I / m_hadronpars[4]);
  double c = -1.0 * I / m_hadronpars[4];

  if (b == 0 && a == 0) {
    B2WARNING("both a and b coefficiants for hadron correction are 0");
    return I;
  }

  double discr = b * b - 4.0 * a * c;
  if (discr < 0) {
    B2WARNING("negative discriminant; return uncorrectecd value");
    return I;
  }

  double D = (a != 0) ? (-b + sqrt(discr)) / (2.0 * a) : -c / b;
  if (D < 0) {
    B2WARNING("D is less 0! will try another solution");
    D = (a != 0) ? (-b - sqrt(discr)) / (2.0 * a) : -c / b;
    if (D < 0) {
      B2WARNING("D is still less 0! just return uncorrectecd value");
      return I;
    }
  }

  return D;
}

//---------------------------------------------------------------------------------------
void HitLevelInfoWriterModule::calculateMeans(double* mean, double* truncMean, double* truncMeanErr,
                                              const vector<double>& dedx) const
{
  // Calculate the truncated average by skipping the lowest & highest
  // events in the array of dE/dx values
  vector<double> sortedDedx = dedx;
  sort(sortedDedx.begin(), sortedDedx.end());
  sortedDedx.erase(remove(sortedDedx.begin(), sortedDedx.end(), 0), sortedDedx.end());
  sortedDedx.shrink_to_fit();

  double truncMeanTmp = 0.0, meanTmp = 0.0, sumOfSqs = 0.0;
  int nValTrunc = 0;
  const int numDedx = sortedDedx.size();

  // add a factor of 0.51 here to make sure we are rounding appropriately...
  const int lEdgeTrunc = int(numDedx * 0.05 + 0.51);
  const int hEdgeTrunc = int(numDedx * (1 - 0.25) + 0.51);
  for (int i = 0; i < numDedx; i++) {
    meanTmp += sortedDedx[i];
    if (i >= lEdgeTrunc and i < hEdgeTrunc) {
      truncMeanTmp += sortedDedx[i];
      sumOfSqs += sortedDedx[i] * sortedDedx[i];
      nValTrunc++;
    }
  }

  if (numDedx != 0) meanTmp /= numDedx;

  if (nValTrunc != 0) truncMeanTmp /= nValTrunc;
  else truncMeanTmp = meanTmp;

  *mean = meanTmp;
  *truncMean = truncMeanTmp;

  if (nValTrunc > 1)
    *truncMeanErr = sqrt(sumOfSqs / double(nValTrunc) - truncMeanTmp * truncMeanTmp) / double(nValTrunc - 1);
  else *truncMeanErr = 0;

}

//---------------------------------------------------------------------------------------
void HitLevelInfoWriterModule::saveChiValue(double(&chi)[Const::ChargedStable::c_SetSize], CDCDedxTrack* dedxTrack,
                                            double dedx) const
{
  // determine a chi value for each particle type
  Const::ParticleSet set = Const::chargedStableSet;
  for (const Const::ChargedStable pdgIter : set) {

    // determine the predicted mean and resolution
    double mean = dedxTrack->getPmean(pdgIter.getIndex());
    double sigma = dedxTrack->getPreso(pdgIter.getIndex());

    // fill the chi value for this particle type
    if (sigma != 0) chi[pdgIter.getIndex()] = ((dedx - mean) / (sigma));
  }
}

//---------------------------------------------------------------------------------------
void HitLevelInfoWriterModule::clearEntries()
{
  for (int il = 0; il < 200; ++il) {
    l_nhitscombined[il] = 0;
    l_wirelongesthit[il] = 0;
    l_layer[il] = 0;
    l_path[il] = 0;
    l_dedx[il] = 0;
  }

  if (m_isHitLevel) {
    for (int ihit = 0; ihit < 200; ++ihit) {
      h_lwire[ihit] = 0;
      h_wire[ihit] = 0;
      h_layer[ihit] = 0;
      h_path[ihit] = 0;
      h_dedx[ihit] = 0;
      h_adcraw[ihit] = 0;
      h_adccorr[ihit] = 0;
      h_doca[ihit] = 0;
      h_ndoca[ihit] = 0;
      h_ndocaRS[ihit] = 0;
      h_enta[ihit] = 0;
      h_entaRS[ihit] = 0;
      h_driftT[ihit] = 0;
      // h_driftD[ihit] = 0;
      h_facnladc[ihit] = 0;
      h_wireGain[ihit] = 0;
      h_twodCor[ihit] = 0;
      h_onedCor[ihit] = 0;
      h_WeightPionHypo[ihit] = 0;
      h_WeightKaonHypo[ihit] = 0;
      h_WeightProtonHypo[ihit] = 0;
      h_foundByTrackFinder[ihit] = 0;
    }
  }
}

//---------------------------------------------------------------------------------------
void HitLevelInfoWriterModule::bookOutput(string filename)
{
  // register output root file
  m_file.push_back(new TFile(filename.c_str(), "RECREATE"));
  m_tree.push_back(new TTree("track", "dE/dx information"));

  int i = m_tree.size() - 1;
  m_tree[i]->SetDirectory(0);

  // event level information (from emd)
  m_tree[i]->Branch("exp", &m_expID, "exp/I");
  m_tree[i]->Branch("run", &m_runID, "run/I");
  m_tree[i]->Branch("event", &m_eventID, "event/I");

  // track level information (from tfr)
  m_tree[i]->Branch("dz", &m_dz, "dz/D");
  m_tree[i]->Branch("dr", &m_dr, "dr/D");
  m_tree[i]->Branch("chi2", &m_chi2, "chi2/D");
  m_tree[i]->Branch("injtime", &m_injtime, "injtime/D");
  m_tree[i]->Branch("isher", &m_injring, "isher/D");

  if (m_isExtraVar) {
    m_tree[i]->Branch("d0", &m_d0, "d0/D");
    m_tree[i]->Branch("z0", &m_z0, "z0/D");
    m_tree[i]->Branch("dphi", &m_dphi, "dphi/D");
    m_tree[i]->Branch("vx0", &m_vx0, "vx0/D");
    m_tree[i]->Branch("vy0", &m_vy0, "vy0/D");
    m_tree[i]->Branch("vz0", &m_vz0, "vz0/D");
    m_tree[i]->Branch("tanlambda", &m_tanlambda, "tanlambda/D");
    m_tree[i]->Branch("phi0", &m_phi0, "phi0/D");
    m_tree[i]->Branch("e1_9", &m_e1_9, "e1_9/D");
    m_tree[i]->Branch("e9_21", &m_e9_21, "e9_21/D");
    m_tree[i]->Branch("eclsnhits", &m_eclsnHits, "eclsnhits/D");
    m_tree[i]->Branch("klmLayers", &m_klmLayers, "klmLayers/I");
  }

  // track level information (from cdt)
  m_tree[i]->Branch("nCDChits", &m_nCDChits, "nCDChits/D");
  m_tree[i]->Branch("inCDC", &m_inCDC, "inCDC/I");
  m_tree[i]->Branch("track", &m_trackID, "track/I");
  m_tree[i]->Branch("length", &m_length, "length/D");
  m_tree[i]->Branch("charge", &m_charge, "charge/I");
  m_tree[i]->Branch("costh", &m_cosTheta, "costh/D");
  m_tree[i]->Branch("pF", &m_pCDC, "pF/D");
  m_tree[i]->Branch("p", &m_p, "p/D");
  m_tree[i]->Branch("pt", &m_pt, "pt/D");
  m_tree[i]->Branch("ioasym", &m_ioasym, "ioasym/D");
  m_tree[i]->Branch("phi", &m_phi, "phi/D");
  m_tree[i]->Branch("pdg", &m_PDG, "pdg/D");
  // track level dE/dx measurements
  m_tree[i]->Branch("mean", &m_mean, "mean/D");
  m_tree[i]->Branch("dedx", &m_trunc, "dedx/D");
  m_tree[i]->Branch("dedxnosat", &m_truncNoSat, "dedxnosat/D");
  m_tree[i]->Branch("dedxerr", &m_error, "dedxerr/D");

  // other track level information
  m_tree[i]->Branch("eop", &m_eop, "eop/D");
  m_tree[i]->Branch("e", &m_e, "e/D");

  // calibration constants
  m_tree[i]->Branch("scale", &m_scale, "scale/D");
  m_tree[i]->Branch("coscor", &m_cosCor, "coscor/D");
  m_tree[i]->Branch("cosedgecor", &m_cosEdgeCor, "cosedgecor/D");
  m_tree[i]->Branch("rungain", &m_runGain, "rungain/D");
  m_tree[i]->Branch("timegain", &m_timeGain, "timegain/D");
  m_tree[i]->Branch("timereso", &m_timeReso, "timereso/D");

  // PID values
  m_tree[i]->Branch("chiE", &m_chie, "chiE/D");
  m_tree[i]->Branch("chiMu", &m_chimu, "chiMu/D");
  m_tree[i]->Branch("chiPi", &m_chipi, "chiPi/D");
  m_tree[i]->Branch("chiK", &m_chik, "chiK/D");
  m_tree[i]->Branch("chiP", &m_chip, "chiP/D");
  m_tree[i]->Branch("chiD", &m_chid, "chiD/D");

  m_tree[i]->Branch("chiEOld", &m_chieOld, "chiEOld/D");
  m_tree[i]->Branch("chiMuOld", &m_chimuOld, "chiMuOld/D");
  m_tree[i]->Branch("chiPiOld", &m_chipiOld, "chiPiOld/D");
  m_tree[i]->Branch("chiKOld", &m_chikOld, "chiKOld/D");
  m_tree[i]->Branch("chiPOld", &m_chipOld, "chiPOld/D");
  m_tree[i]->Branch("chiDOld", &m_chidOld, "chiDOld/D");

  m_tree[i]->Branch("pmeanE", &m_pmeane, "pmeanE/D");
  m_tree[i]->Branch("pmeanMu", &m_pmeanmu, "pmeanMu/D");
  m_tree[i]->Branch("pmeanPi", &m_pmeanpi, "pmeanPi/D");
  m_tree[i]->Branch("pmeanK", &m_pmeank, "pmeanK/D");
  m_tree[i]->Branch("pmeanP", &m_pmeanp, "pmeanP/D");
  m_tree[i]->Branch("pmeanD", &m_pmeand, "pmeanD/D");

  m_tree[i]->Branch("presE", &m_prese, "presE/D");
  m_tree[i]->Branch("presMu", &m_presmu, "presMu/D");
  m_tree[i]->Branch("presPi", &m_prespi, "presPi/D");
  m_tree[i]->Branch("presK", &m_presk, "presK/D");
  m_tree[i]->Branch("presP", &m_presp, "presP/D");
  m_tree[i]->Branch("presD", &m_presd, "presD/D");

  // layer level information
  m_tree[i]->Branch("lNHits", &l_nhits, "lNHits/I");
  m_tree[i]->Branch("lNHitsUsed", &l_nhitsused, "lNHitsUsed/I");
  m_tree[i]->Branch("lNHitsCombined", l_nhitscombined, "lNHitsCombined[lNHits]/I");
  m_tree[i]->Branch("lWireLongestHit", l_wirelongesthit, "lWireLongestHit[lNHits]/I");
  m_tree[i]->Branch("lLayer", l_layer, "lLayer[lNHits]/I");
  m_tree[i]->Branch("lPath", l_path, "lPath[lNHits]/D");
  m_tree[i]->Branch("lDedx", l_dedx, "lDedx[lNHits]/D");

  // hit level information
  if (m_isHitLevel) {
    m_tree[i]->Branch("hNHits", &h_nhits, "hNHits/I");
    m_tree[i]->Branch("hLWire", h_lwire, "hLWire[hNHits]/I");
    m_tree[i]->Branch("hWire", h_wire, "hWire[hNHits]/I");
    m_tree[i]->Branch("hLayer", h_layer, "hLayer[hNHits]/I");
    m_tree[i]->Branch("hPath", h_path, "hPath[hNHits]/D");
    m_tree[i]->Branch("hDedx", h_dedx, "hDedx[hNHits]/D");
    m_tree[i]->Branch("hADCRaw", h_adcraw, "hADCRaw[hNHits]/D");
    m_tree[i]->Branch("hADCCorr", h_adccorr, "hADCCorr[hNHits]/D");
    m_tree[i]->Branch("hDoca", h_doca, "hDoca[hNHits]/D");
    m_tree[i]->Branch("hNDoca", h_ndoca, "hNDoca[hNHits]/D");
    m_tree[i]->Branch("hNDocaRS", h_ndocaRS, "hNDocaRS[hNHits]/D");
    m_tree[i]->Branch("hEnta", h_enta, "hEnta[hNHits]/D");
    m_tree[i]->Branch("hEntaRS", h_entaRS, "hEntaRS[hNHits]/D");
    m_tree[i]->Branch("hDriftT", h_driftT, "hDriftT[hNHits]/D");
    m_tree[i]->Branch("hDriftD", h_driftD, "hDriftD[hNHits]/D");
    m_tree[i]->Branch("hFacnlADC", h_facnladc, "hFacnlADC[hNHits]/D");
    m_tree[i]->Branch("hWireGain", h_wireGain, "hWireGain[hNHits]/D");
    m_tree[i]->Branch("hTwodcor", h_twodCor, "hTwodcor[hNHits]/D");
    m_tree[i]->Branch("hOnedcor", h_onedCor, "hOnedcor[hNHits]/D");

    if (m_isExtraVar) {
      m_tree[i]->Branch("hWeightPionHypo", h_WeightPionHypo, "hWeightPionHypo[hNHits]/D");
      m_tree[i]->Branch("hWeightKaonHypo", h_WeightKaonHypo, "hWeightKaonHypo[hNHits]/D");
      m_tree[i]->Branch("hWeightProtonHypo", h_WeightProtonHypo, "hWeightProtonHypo[hNHits]/D");
      m_tree[i]->Branch("hFoundByTrackFinder", h_foundByTrackFinder, "hFoundByTrackFinder[hNHits]/I");
    }
  }
}