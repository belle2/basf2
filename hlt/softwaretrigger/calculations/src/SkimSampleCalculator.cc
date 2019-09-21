/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua Li, Chris Hearty, Nils Braun                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <hlt/softwaretrigger/calculations/SkimSampleCalculator.h>
#include <hlt/softwaretrigger/calculations/utilities.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/ClusterUtility/ClusterUtils.h>
#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBArray.h>
#include <framework/core/Module.h>
#include <reconstruction/dataobjects/CDCDedxTrack.h>
#include <genfit/Track.h>
#include <numeric>
#include <iostream>

using namespace Belle2;
using namespace SoftwareTrigger;

SkimSampleCalculator::SkimSampleCalculator() :
  m_pionParticles("pi+:skim"), m_gammaParticles("gamma:skim")
{

}

void SkimSampleCalculator::requireStoreArrays()
{
  m_pionParticles.isRequired();
  m_gammaParticles.isRequired();
};

void SkimSampleCalculator::doCalculation(SoftwareTriggerObject& calculationResult)
{
  // Prefetch some later needed objects/values
  const Particle* gammaWithMaximumRho = getElementWithMaximumRho<Particle>(m_gammaParticles);
  const Particle* gammaWithSecondMaximumRho = getElementWithMaximumRhoBelow<Particle>(m_gammaParticles,
                                              getRho(gammaWithMaximumRho));
  const Particle* trackWithMaximumRho = getElementWithMaximumRho<Particle>(m_pionParticles);
  const Particle* trackWithSecondMaximumRho = getElementWithMaximumRhoBelow<Particle>(m_pionParticles,
                                              getRho(trackWithMaximumRho));

  const double& rhoOfECLClusterWithMaximumRho = getRhoOfECLClusterWithMaximumRho(m_pionParticles, m_gammaParticles);
  const double& rhoOfECLClusterWithSecondMaximumRho = getRhoOfECLClusterWithMaximumRhoBelow(m_pionParticles,
                                                      m_gammaParticles,
                                                      rhoOfECLClusterWithMaximumRho);

  const double& rhoOfTrackWithMaximumRho = getRho(trackWithMaximumRho);
  const double& rhoOfTrackWithSecondMaximumRho = getRho(trackWithSecondMaximumRho);
  const double& rhoOfGammaWithMaximumRho = getRho(gammaWithMaximumRho);
  const double& rhoOfGammaWithSecondMaximumRho = getRho(gammaWithSecondMaximumRho);

  // Simple to calculate variables
  // EC1CMSLE
  calculationResult["EC1CMSLE"] = rhoOfECLClusterWithMaximumRho;

  // EC2CMSLE
  calculationResult["EC2CMSLE"] = rhoOfECLClusterWithSecondMaximumRho;

  // EC12CMSLE
  calculationResult["EC12CMSLE"] = rhoOfECLClusterWithMaximumRho + rhoOfECLClusterWithSecondMaximumRho;

  // nTracksLE
  calculationResult["nTracksLE"] = m_pionParticles->getListSize();

  // nTracksLE
  calculationResult["nGammasLE"] = m_gammaParticles->getListSize();

  // P1CMSBhabhaLE
  calculationResult["P1CMSBhabhaLE"] = rhoOfTrackWithMaximumRho;

  // P1CMSBhabhaLE/E_beam
  calculationResult["P1OEbeamCMSBhabhaLE"] = rhoOfTrackWithMaximumRho / BeamEnergyCMS();

  // P2CMSBhabhaLE
  calculationResult["P2CMSBhabhaLE"] = rhoOfTrackWithSecondMaximumRho;

  // P2CMSBhabhaLE/E_beam
  calculationResult["P2OEbeamCMSBhabhaLE"] = rhoOfTrackWithSecondMaximumRho / BeamEnergyCMS();

  // P12CMSBhabhaLE
  calculationResult["P12CMSBhabhaLE"] = rhoOfTrackWithMaximumRho + rhoOfTrackWithSecondMaximumRho;

  //G1CMSLE, the largest energy of gamma in CMS
  calculationResult["G1CMSBhabhaLE"] = rhoOfGammaWithMaximumRho;
  //G1OEbeamCMSLE, the largest energy of gamma in CMS over beam energy
  calculationResult["G1OEbeamCMSBhabhaLE"] = rhoOfGammaWithMaximumRho / BeamEnergyCMS();

  //G2CMSLE, the secondary largest energy of gamma in CMS
  calculationResult["G2CMSBhabhaLE"] = rhoOfGammaWithSecondMaximumRho;
  //G2OEbeamCMSLE, the largest energy of gamma in CMS over beam energy
  calculationResult["G2OEbeamCMSBhabhaLE"] = rhoOfGammaWithSecondMaximumRho / BeamEnergyCMS();

  //G12CMSLE, the secondary largest energy of gamma in CMS
  calculationResult["G12CMSBhabhaLE"] = rhoOfGammaWithMaximumRho + rhoOfGammaWithSecondMaximumRho;
  //G12CMSLE, the secondary largest energy of gamma in CMS over beam energy
  calculationResult["G12OEbeamCMSBhabhaLE"] =
    (rhoOfGammaWithMaximumRho + rhoOfGammaWithSecondMaximumRho) / BeamEnergyCMS();


  // Medium hard to calculate variables
  // ENeutralLE
  if (gammaWithMaximumRho) {
    calculationResult["ENeutralLE"] = getRho(gammaWithMaximumRho);
  } else {
    calculationResult["ENeutralLE"] = -1;
  }

  // nECLMatchTracksLE
  const unsigned int numberOfTracksWithECLMatch = std::count_if(m_pionParticles->begin(), m_pionParticles->end(),
  [](const Particle & particle) {
    return particle.getECLCluster() != nullptr;
  });
  calculationResult["nECLMatchTracksLE"] = numberOfTracksWithECLMatch;

  //nECLClustersLE
  double neclClusters = -1.;
  StoreArray<ECLCluster> eclClusters;
  if (eclClusters.isValid()) {
    const unsigned int numberOfECLClusters = std::count_if(eclClusters.begin(), eclClusters.end(),
    [](const ECLCluster & eclcluster) {
      return (eclcluster.hasHypothesis(
                ECLCluster::EHypothesisBit::c_nPhotons)
              and eclcluster.getEnergy(
                ECLCluster::EHypothesisBit::c_nPhotons) > 0.1);
    });
    neclClusters = numberOfECLClusters;
  }
  calculationResult["nECLClustersLE"] = neclClusters;

  int nb2bcc_PhiHigh = 0;
  int nb2bcc_PhiLow = 0;
  int nb2bcc_3D = 0;
  ClusterUtils C;
  for (int i = 0; i < eclClusters.getEntries() - 1; i++) {
    if (!eclClusters[i]->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons))
      continue;
    TLorentzVector V4g1 = C.Get4MomentumFromCluster(eclClusters[i], ECLCluster::EHypothesisBit::c_nPhotons);
    double Eg1 = V4g1.E();
    for (int j = i + 1; j < eclClusters.getEntries(); j++) {
      if (!eclClusters[j]->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons))
        continue;
      TLorentzVector V4g2 = C.Get4MomentumFromCluster(eclClusters[j], ECLCluster::EHypothesisBit::c_nPhotons);
      double Eg2 = V4g2.E();
      const TVector3 V3g1 = (PCmsLabTransform::labToCms(V4g1)).Vect();
      const TVector3 V3g2 = (PCmsLabTransform::labToCms(V4g2)).Vect();
      double Thetag1 = (PCmsLabTransform::labToCms(V4g1)).Theta() * 180. / 3.1415926;
      double Thetag2 = (PCmsLabTransform::labToCms(V4g2)).Theta() * 180. / 3.1415926;
      double deltphi = fabs(V3g1.DeltaPhi(V3g2) * 180. / 3.1415926);
      double Tsum = Thetag1 + Thetag2;
      if (deltphi > 170. && (Eg1 > 0.25 && Eg2 > 0.25)) nb2bcc_PhiHigh++;
      if (deltphi > 170. && (Eg1 < 0.25 || Eg2 < 0.25)) nb2bcc_PhiLow++;
      if (deltphi > 160. && (Tsum > 160. && Tsum < 200.)) nb2bcc_3D++;
    }
  }

  calculationResult["nB2BCCPhiHighLE"] = nb2bcc_PhiHigh;
  calculationResult["nB2BCCPhiLowLE"] = nb2bcc_PhiLow;
  calculationResult["nB2BCC3DLE"] = nb2bcc_3D;


  // AngleGTLE
  double angleGTLE = -10.;
  if (gammaWithMaximumRho) {
    const TLorentzVector& V4g1 = gammaWithMaximumRho->get4Vector();
    if (trackWithMaximumRho) {
      const TLorentzVector& V4p1 = trackWithMaximumRho->get4Vector();
      const double theta1 = (V4g1.Vect()).Angle(V4p1.Vect());
      if (angleGTLE < theta1) angleGTLE = theta1;
    }
    if (trackWithSecondMaximumRho) {
      const TLorentzVector& V4p2 = trackWithSecondMaximumRho->get4Vector();
      const double theta2 = (V4g1.Vect()).Angle(V4p2.Vect());
      if (angleGTLE < theta2) angleGTLE = theta2;
    }
  }

  calculationResult["AngleGTLE"] = angleGTLE;


  // AngleG1G2LE
  double angleG1G2CMSLE = -10.;
  if (gammaWithMaximumRho) {
    const TLorentzVector& V4p1 = gammaWithMaximumRho->get4Vector();
    if (gammaWithSecondMaximumRho) {
      const TLorentzVector& V4p2 = gammaWithSecondMaximumRho->get4Vector();
      const TVector3 V3p1 = (PCmsLabTransform::labToCms(V4p1)).Vect();
      const TVector3 V3p2 = (PCmsLabTransform::labToCms(V4p2)).Vect();
      angleG1G2CMSLE = V3p1.Angle(V3p2);
    }
  }

  calculationResult["AngleG1G2CMSLE"] = angleG1G2CMSLE;

  // maxAngleTTLE
  double maxAngleTTLE = -10.;
  if (m_pionParticles->getListSize() >= 2) {
    for (unsigned int i = 0; i < m_pionParticles->getListSize() - 1; i++) {
      Particle* par1 = m_pionParticles->getParticle(i);
      for (unsigned int j = i + 1; j < m_pionParticles->getListSize(); j++) {
        Particle* par2 = m_pionParticles->getParticle(j);
        TLorentzVector V4p1 = par1->get4Vector();
        TLorentzVector V4p2 = par2->get4Vector();
        const TVector3 V3p1 = (PCmsLabTransform::labToCms(V4p1)).Vect();
        const TVector3 V3p2 = (PCmsLabTransform::labToCms(V4p2)).Vect();
        const double temp = V3p1.Angle(V3p2);
        if (maxAngleTTLE < temp) maxAngleTTLE = temp;
      }
    }
  }

  calculationResult["maxAngleTTLE"] = maxAngleTTLE;

  //maxAngleGGLE

  double maxAngleGGLE = -10.;
  if (m_gammaParticles->getListSize() >= 2) {
    for (unsigned int i = 0; i < m_gammaParticles->getListSize() - 1; i++) {
      Particle* par1 = m_gammaParticles->getParticle(i);
      for (unsigned int j = i + 1; j < m_gammaParticles->getListSize(); j++) {
        Particle* par2 = m_gammaParticles->getParticle(j);
        TLorentzVector V4p1 = par1->get4Vector();
        TLorentzVector V4p2 = par2->get4Vector();
        const TVector3 V3p1 = (PCmsLabTransform::labToCms(V4p1)).Vect();
        const TVector3 V3p2 = (PCmsLabTransform::labToCms(V4p2)).Vect();
        const double temp = V3p1.Angle(V3p2);
        if (maxAngleGGLE < temp) maxAngleGGLE = temp;
      }
    }
  }

  calculationResult["maxAngleGGLE"] = maxAngleGGLE;

  // nEidLE
  const unsigned int nEidLE = std::count_if(m_pionParticles->begin(), m_pionParticles->end(),
  [](const Particle & p) {
    const double& momentum = p.getMomentumMagnitude();
    const double& r_rho = getRho(&p);
    const ECLCluster* eclTrack = p.getECLCluster();
    if (eclTrack) {
      const double& energyOverMomentum = eclTrack->getEnergy(
                                           ECLCluster::EHypothesisBit::c_nPhotons) / momentum;
      double r_rhotoebeam = r_rho / BeamEnergyCMS();
      return (r_rhotoebeam) > 0.35 && energyOverMomentum > 0.8;
    }
    return false;
  });

  calculationResult["nEidLE"] = nEidLE;


  // VisibleEnergyLE
  const double visibleEnergyTracks = std::accumulate(m_pionParticles->begin(), m_pionParticles->end(), 0.0,
  [](const double & visibleEnergy, const Particle & p) {
    return visibleEnergy + p.getMomentumMagnitude();
  });

  const double visibleEnergyGammas = std::accumulate(m_gammaParticles->begin(), m_gammaParticles->end(), 0.0,
  [](const double & visibleEnergy, const Particle & p) {
    return visibleEnergy + p.getMomentumMagnitude();
  });

  calculationResult["VisibleEnergyLE"] = visibleEnergyTracks + visibleEnergyGammas;

  // EtotLE
  const double eTotTracks = std::accumulate(m_pionParticles->begin(), m_pionParticles->end(), 0.0,
  [](const double & eTot, const Particle & p) {
    const ECLCluster* eclCluster = p.getECLCluster();
    if (eclCluster) {
      const double eclEnergy = eclCluster->getEnergy(
                                 ECLCluster::EHypothesisBit::c_nPhotons);
      if (eclEnergy > 0.1) {
        return eTot + eclCluster->getEnergy(
                 ECLCluster::EHypothesisBit::c_nPhotons);
      }
    }
    return eTot;
  });

  const double eTotGammas = std::accumulate(m_gammaParticles->begin(), m_gammaParticles->end(), 0.0,
  [](const double & eTot, const Particle & p) {
    return eTot + p.getEnergy();
  });
  double Etot = eTotTracks + eTotGammas;
  calculationResult["EtotLE"] = Etot;

  //KLM inforamtion
  // The clusters with the largest pentrate layers in KLM.
  double numMaxLayerKLM = -1.;
  double numSecMaxLayerKLM = -1.;
  StoreArray<KLMCluster> klmClusters;
  if (klmClusters.isValid()) {
    for (const auto& klmCluster : klmClusters) {
      double klmClusterLayer = klmCluster.getLayers();
      if (numMaxLayerKLM < klmClusterLayer) {
        numSecMaxLayerKLM = numMaxLayerKLM;
        numMaxLayerKLM = klmClusterLayer;
      } else if (numSecMaxLayerKLM < klmClusterLayer)
        numSecMaxLayerKLM = klmClusterLayer;
    }
  }
  calculationResult["N1KLMLayer"] = numMaxLayerKLM;
  calculationResult["N2KLMLayer"] = numSecMaxLayerKLM;

  //define bhabha_2trk, bhabha_1trk, eclbhabha
  int charget1 = -10;
  if (trackWithMaximumRho) charget1 = trackWithMaximumRho->getCharge();
  int charget2 = -10;
  if (trackWithSecondMaximumRho) charget2 = trackWithSecondMaximumRho->getCharge();

  double Bhabha2Trk = 0.;
  int ntrk_bha = m_pionParticles->getListSize();
  double rp1ob = rhoOfTrackWithMaximumRho / BeamEnergyCMS();
  double rp2ob = rhoOfTrackWithSecondMaximumRho / BeamEnergyCMS();
  bool bhabha2trk_tag =
    ntrk_bha >= 2 && maxAngleTTLE > 2.88 && nEidLE >= 1 && rp1ob > 0.35 && rp2ob > 0.35 && (Etot) > 4.0
    && (abs(charget1) == 1 && abs(charget2) == 1 && (charget1 + charget2) == 0);
  if (bhabha2trk_tag) Bhabha2Trk = 1;
  calculationResult["Bhabha2Trk"] = Bhabha2Trk;

  double Bhabha1Trk = 0.;
  double rc1ob = rhoOfGammaWithMaximumRho / BeamEnergyCMS();
  double rc2ob = rhoOfGammaWithSecondMaximumRho / BeamEnergyCMS();
  bool bhabha1trk_tag = ntrk_bha == 1 && rp1ob > 0.35 && rc1ob > 0.35 && angleGTLE > 2.618;
  if (bhabha1trk_tag) Bhabha1Trk = 1;
  calculationResult["Bhabha1Trk"] = Bhabha1Trk;

  double ggSel = 0.;
  bool gg_tag = ntrk_bha <= 1 && nEidLE == 0 && rc1ob > 0.35 && rc2ob > 0.2 && Etot > 4.0 && maxAngleGGLE > 2.618;
  if (gg_tag) ggSel = 1;
  calculationResult["GG"] = ggSel;

  // Bhabha skim with ECL information only (bhabhaecl)
  double BhabhaECL = 0.;
  ClusterUtils Cls;
  for (int i = 0; i < eclClusters.getEntries() - 1; i++) {
    if (!eclClusters[i]->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons))
      continue;

    TLorentzVector V4g1 = PCmsLabTransform::labToCms(Cls.Get4MomentumFromCluster(eclClusters[i],
                                                     ECLCluster::EHypothesisBit::c_nPhotons));
    double Eg1ob = V4g1.E() / (2 * BeamEnergyCMS());
    for (int j = i + 1; j < eclClusters.getEntries(); j++) {
      if (!eclClusters[j]->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons))
        continue;
      TLorentzVector V4g2 = PCmsLabTransform::labToCms(Cls.Get4MomentumFromCluster(eclClusters[j],
                                                       ECLCluster::EHypothesisBit::c_nPhotons));
      double Eg2ob = V4g2.E() / (2 * BeamEnergyCMS());
      const TVector3 V3g1 = V4g1.Vect();
      const TVector3 V3g2 = V4g2.Vect();
      double Thetag1 = V4g1.Theta() * TMath::RadToDeg();
      double Thetag2 = V4g2.Theta() * TMath::RadToDeg();
      double deltphi = fabs(V3g1.DeltaPhi(V3g2) * TMath::RadToDeg());
      double Tsum = Thetag1 + Thetag2;
      if ((deltphi > 165. && deltphi < 178.5) && (Eg1ob > 0.4 && Eg2ob > 0.4 && (Eg1ob > 0.45 || Eg2ob > 0.45)) && (Tsum > 178.
          && Tsum < 182.)) BhabhaECL = 1;
    }
  }
  calculationResult["BhabhaECL"] = BhabhaECL;

  // Radiative Bhabha skim (radee) for CDC dE/dx calib studies
  double radee = 0.;
  const double lowdEdxEdge = 0.8, highdEdxEdge = 1.2;
  const double lowEoPEdge = 0.8, highEoPEdge = 1.2;

  if (m_pionParticles->getListSize() == 2) {

    //------------First track variables----------------
    for (unsigned int i = 0; i < m_pionParticles->getListSize() - 1; i++) {

      Particle* part1 = m_pionParticles->getParticle(i);
      if (!part1) continue;

      const auto chargep1 = part1->getCharge();
      if (abs(chargep1) != 1) continue;

      const ECLCluster* eclTrack1 = part1->getECLCluster();
      if (!eclTrack1) continue;
      if (!eclTrack1->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) continue;

      const double& momentum1 = part1->getMomentumMagnitude();
      const double& energyOverMomentum1 = eclTrack1->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons) / momentum1;
      if (energyOverMomentum1 <= lowEoPEdge || energyOverMomentum1 >= highEoPEdge) continue;

      const Track* track1 = part1->getTrack();
      if (!track1) continue;

      const TrackFitResult* trackFit1 = track1->getTrackFitResultWithClosestMass(Const::pion);
      if (!trackFit1) continue;
      if (trackFit1->getHitPatternCDC().getNHits() <= 0) continue;

      const CDCDedxTrack* dedxTrack1 = track1->getRelatedTo<CDCDedxTrack>();
      if (!dedxTrack1) continue;

      //------------Second track variables----------------
      for (unsigned int j = i + 1; j < m_pionParticles->getListSize(); j++) {

        Particle* part2 = m_pionParticles->getParticle(j);
        if (!part2) continue;

        const auto chargep2 = part2->getCharge();
        if (abs(chargep2) != 1 || (chargep1 + chargep2 != 0)) continue;

        const ECLCluster* eclTrack2 = part2->getECLCluster();
        if (!eclTrack2) continue;
        if (!eclTrack2->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) continue;

        const double& momentum2 = part2->getMomentumMagnitude();
        const double& energyOverMomentum2 = eclTrack2->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons) / momentum2;
        if (energyOverMomentum2 <= lowEoPEdge || energyOverMomentum2 >= highEoPEdge) continue;

        const Track* track2 = part2->getTrack();
        if (!track2) continue;

        const TrackFitResult* trackFit2 = track2->getTrackFitResultWithClosestMass(Const::pion);
        if (!trackFit2) continue;
        if (trackFit2->getHitPatternCDC().getNHits() <= 0) continue;

        CDCDedxTrack* dedxTrack2 = track2->getRelatedTo<CDCDedxTrack>();
        if (!dedxTrack2) continue;

        double p1_dedxnosat = dedxTrack1->getDedxNoSat();
        double p2_dedxnosat = dedxTrack2->getDedxNoSat();

        if ((p1_dedxnosat > lowdEdxEdge && p1_dedxnosat < highdEdxEdge)  || (p2_dedxnosat > lowdEdxEdge
            && p2_dedxnosat < highdEdxEdge))radee = 1;

      }
    }
  }

  calculationResult["Radee"] = radee;

}
