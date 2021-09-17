/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <hlt/softwaretrigger/calculations/SkimSampleCalculator.h>

#include <hlt/softwaretrigger/calculations/utilities.h>

#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ReferenceFrame.h>

#include <analysis/ClusterUtility/ClusterUtils.h>
#include <analysis/ContinuumSuppression/FoxWolfram.h>
#include <analysis/dataobjects/Particle.h>

#include <analysis/variables/AcceptanceVariables.h>
#include <analysis/variables/BelleVariables.h>
#include <analysis/variables/ECLVariables.h>
#include <analysis/variables/FlightInfoVariables.h>

#include <framework/geometry/B2Vector3.h>

#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <reconstruction/dataobjects/CDCDedxTrack.h>

#include <numeric>
#include <TDatabasePDG.h>

using namespace Belle2;
using namespace SoftwareTrigger;

SkimSampleCalculator::SkimSampleCalculator() :
  m_pionParticles("pi+:skim"), m_gammaParticles("gamma:skim"), m_pionHadParticles("pi+:hadb"), m_pionTauParticles("pi+:tau"),
  m_KsParticles("K_S0:merged"), m_LambdaParticles("Lambda0:merged"), m_DstParticles("D*+:d0pi"), m_offIpParticles("pi+:offip"),
  m_filterL1TrgNN("software_trigger_cut&filter&L1_trigger_nn_info")
{

}

void SkimSampleCalculator::requireStoreArrays()
{
  m_pionParticles.isRequired();
  m_gammaParticles.isRequired();
  m_pionHadParticles.isRequired();
  m_pionTauParticles.isRequired();
  m_KsParticles.isOptional();
  m_LambdaParticles.isOptional();
  m_DstParticles.isOptional();
  m_offIpParticles.isRequired();

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

  // nTracksTAU
  calculationResult["nTracksTAU"] = m_pionTauParticles->getListSize();

  // nGammasLE
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
  double eneclClusters = 0.;
  StoreArray<ECLCluster> eclClusters;
  ClusterUtils Cl;
  double PzGamma = 0.;
  double EsumGamma = 0.;
  if (eclClusters.isValid()) {
    const unsigned int numberOfECLClusters = std::count_if(eclClusters.begin(), eclClusters.end(),
    [](const ECLCluster & eclcluster) {
      return (eclcluster.hasHypothesis(
                ECLCluster::EHypothesisBit::c_nPhotons)
              and eclcluster.getEnergy(
                ECLCluster::EHypothesisBit::c_nPhotons) > 0.1);
    });
    neclClusters = numberOfECLClusters;

    for (int ncl = 0; ncl < eclClusters.getEntries(); ncl++) {
      if (eclClusters[ncl]->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)
          && eclClusters[ncl]->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons) > 0.1) {
        eneclClusters += eclClusters[ncl]->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons);
        if (!eclClusters[ncl]->getRelatedFrom<Track>()) {
          ROOT::Math::PxPyPzEVector V4Gamma_CMS = PCmsLabTransform::labToCms(Cl.Get4MomentumFromCluster(eclClusters[ncl],
                                                  ECLCluster::EHypothesisBit::c_nPhotons));
          EsumGamma += V4Gamma_CMS.E();
          PzGamma += V4Gamma_CMS.Pz();
        }
      }
    }
  }
  calculationResult["nECLClustersLE"] = neclClusters;

  int nb2bcc_PhiHigh = 0;
  int nb2bcc_PhiLow = 0;
  int nb2bcc_3D = 0;
  ClusterUtils C;
  for (int i = 0; i < eclClusters.getEntries() - 1; i++) {
    if (!eclClusters[i]->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons))
      continue;
    ROOT::Math::PxPyPzEVector V4g1 = C.Get4MomentumFromCluster(eclClusters[i], ECLCluster::EHypothesisBit::c_nPhotons);
    double Eg1 = V4g1.E();
    for (int j = i + 1; j < eclClusters.getEntries(); j++) {
      if (!eclClusters[j]->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons))
        continue;
      ROOT::Math::PxPyPzEVector V4g2 = C.Get4MomentumFromCluster(eclClusters[j], ECLCluster::EHypothesisBit::c_nPhotons);
      double Eg2 = V4g2.E();
      const B2Vector3D V3g1 = (PCmsLabTransform::labToCms(V4g1)).Vect();
      const B2Vector3D V3g2 = (PCmsLabTransform::labToCms(V4g2)).Vect();
      double Thetag1 = (PCmsLabTransform::labToCms(V4g1)).Theta() * TMath::RadToDeg();
      double Thetag2 = (PCmsLabTransform::labToCms(V4g2)).Theta() * TMath::RadToDeg();
      double deltphi = fabs(V3g1.DeltaPhi(V3g2) * TMath::RadToDeg());
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
    const B2Vector3D& V3g1 = gammaWithMaximumRho->getMomentum();
    if (trackWithMaximumRho) {
      const B2Vector3D& V3p1 = trackWithMaximumRho->getMomentum();
      const double theta1 = V3g1.Angle(V3p1);
      if (angleGTLE < theta1) angleGTLE = theta1;
    }
    if (trackWithSecondMaximumRho) {
      const B2Vector3D& V3p2 = trackWithSecondMaximumRho->getMomentum();
      const double theta2 = V3g1.Angle(V3p2);
      if (angleGTLE < theta2) angleGTLE = theta2;
    }
  }

  calculationResult["AngleGTLE"] = angleGTLE;

  // AngleG1G2LE
  double angleG1G2CMSLE = -10.;
  if (gammaWithMaximumRho) {
    const ROOT::Math::PxPyPzEVector& V4p1 = gammaWithMaximumRho->get4Vector();
    if (gammaWithSecondMaximumRho) {
      const ROOT::Math::PxPyPzEVector& V4p2 = gammaWithSecondMaximumRho->get4Vector();
      const B2Vector3D V3p1 = (PCmsLabTransform::labToCms(V4p1)).Vect();
      const B2Vector3D V3p2 = (PCmsLabTransform::labToCms(V4p2)).Vect();
      angleG1G2CMSLE = V3p1.Angle(V3p2);
    }
  }

  calculationResult["AngleG1G2CMSLE"] = angleG1G2CMSLE;

  // maxAngleTTLE
  double maxAngleTTLE = -10.;
  int nJpsi = 0;
  double Jpsi = 0.;
  const double jPsiMasswindow = 0.11;
  if (m_pionParticles->getListSize() >= 2) {
    for (unsigned int i = 0; i < m_pionParticles->getListSize() - 1; i++) {
      Particle* par1 = m_pionParticles->getParticle(i);
      for (unsigned int j = i + 1; j < m_pionParticles->getListSize(); j++) {
        Particle* par2 = m_pionParticles->getParticle(j);
        ROOT::Math::PxPyPzEVector V4p1 = par1->get4Vector();
        ROOT::Math::PxPyPzEVector V4p2 = par2->get4Vector();
        ROOT::Math::PxPyPzEVector V4pSum = V4p1 + V4p2;
        const auto chSum = par1->getCharge() + par2->getCharge();
        const double mSum = V4pSum.M();
        const double JpsidM = mSum - TDatabasePDG::Instance()->GetParticle(443)->Mass();
        if (abs(JpsidM) < jPsiMasswindow && chSum == 0)  nJpsi++;
        const B2Vector3D V3p1 = (PCmsLabTransform::labToCms(V4p1)).Vect();
        const B2Vector3D V3p2 = (PCmsLabTransform::labToCms(V4p2)).Vect();
        const double temp = V3p1.Angle(V3p2);
        if (maxAngleTTLE < temp) maxAngleTTLE = temp;
      }
    }
  }

  if (nJpsi != 0) Jpsi = 1;

  calculationResult["maxAngleTTLE"] = maxAngleTTLE;
  calculationResult["Jpsi"] = Jpsi;

  //maxAngleGGLE
  double maxAngleGGLE = -10.;
  if (m_gammaParticles->getListSize() >= 2) {
    for (unsigned int i = 0; i < m_gammaParticles->getListSize() - 1; i++) {
      Particle* par1 = m_gammaParticles->getParticle(i);
      for (unsigned int j = i + 1; j < m_gammaParticles->getListSize(); j++) {
        Particle* par2 = m_gammaParticles->getParticle(j);
        ROOT::Math::PxPyPzEVector V4p1 = par1->get4Vector();
        ROOT::Math::PxPyPzEVector V4p2 = par2->get4Vector();
        const B2Vector3D V3p1 = (PCmsLabTransform::labToCms(V4p1)).Vect();
        const B2Vector3D V3p2 = (PCmsLabTransform::labToCms(V4p2)).Vect();
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

    ROOT::Math::PxPyPzEVector V4g1 = PCmsLabTransform::labToCms(Cls.Get4MomentumFromCluster(eclClusters[i],
                                                                ECLCluster::EHypothesisBit::c_nPhotons));
    double Eg1ob = V4g1.E() / (2 * BeamEnergyCMS());
    for (int j = i + 1; j < eclClusters.getEntries(); j++) {
      if (!eclClusters[j]->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons))
        continue;
      ROOT::Math::PxPyPzEVector V4g2 = PCmsLabTransform::labToCms(Cls.Get4MomentumFromCluster(eclClusters[j],
                                                                  ECLCluster::EHypothesisBit::c_nPhotons));
      double Eg2ob = V4g2.E() / (2 * BeamEnergyCMS());
      const B2Vector3D V3g1 = V4g1.Vect();
      const B2Vector3D V3g2 = V4g2.Vect();
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
  const double lowdEdxEdge = 0.70, highdEdxEdge = 1.30;
  const double lowEoPEdge = 0.70, highEoPEdge = 1.30;

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

  // Dimuon skim (mumutight) taken from the offline skim + Radiative dimuon (radmumu)
  double mumutight = 0.;
  double eMumuTotGammas = 0.;
  int nTracks = 0;
  double radmumu = 0.;
  const double maxEoP = 0.4;
  int nGammas = m_gammaParticles->getListSize();

  for (int t = 0; t < nGammas; t++) {
    const Particle* part = m_gammaParticles->getParticle(t);
    const auto& frame = ReferenceFrame::GetCurrent();
    eMumuTotGammas += frame.getMomentum(part).E();
  }

  StoreArray<Track> tracks;
  nTracks = tracks.getEntries();
  PCmsLabTransform T;
  const ROOT::Math::PxPyPzEVector pIN = T.getBeamFourMomentum();
  const auto& fr = ReferenceFrame::GetCurrent();

  if (m_pionParticles->getListSize() == 2) {

    //------------First track variables----------------
    for (unsigned int k = 0; k < m_pionParticles->getListSize() - 1; k++) {

      Particle* part1 = m_pionParticles->getParticle(k);
      if (!part1) continue;

      const auto chargep1 = part1->getCharge();
      if (abs(chargep1) != 1) continue;

      const ECLCluster* eclTrack1 = part1->getECLCluster();
      if (!eclTrack1) continue;
      if (!eclTrack1->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) continue;

      const Track* track1 = part1->getTrack();
      if (!track1) continue;

      const TrackFitResult* trackFit1 = track1->getTrackFitResultWithClosestMass(Const::pion);
      if (!trackFit1) continue;

      const ROOT::Math::PxPyPzEVector V4p1 = trackFit1->get4Momentum();
      const B2Vector3D V3p1 = (PCmsLabTransform::labToCms(V4p1)).Vect();

      const double p1MomLab = V4p1.P();
      double highestP = p1MomLab;
      const double p1Eop = Variable::eclClusterEoP(part1);
      const double p1CDChits = trackFit1->getHitPatternCDC().getNHits();
      const PIDLikelihood* p1Pid = part1->getPIDLikelihood();
      bool p1hasKLMid = 0;
      if (p1Pid) p1hasKLMid = p1Pid->isAvailable(Const::KLM);
      const double p1isInCDC = Variable::inCDCAcceptance(part1);
      const double p1clusPhi = Variable::eclClusterPhi(part1);

      const double Pp1 = V3p1.Mag();
      const double Thetap1 = (V3p1).Theta() * TMath::RadToDeg();
      const double Phip1 = (V3p1).Phi() * TMath::RadToDeg();

      const double enECLTrack1 = eclTrack1->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons);

      const bool goodTrk1 = enECLTrack1 > 0 && enECLTrack1 < 0.4 && p1Eop < maxEoP && p1CDChits > 0
                            && ((p1hasKLMid == 0 && enECLTrack1 < 0.25 && p1MomLab < 2.0) || p1hasKLMid == 1) && p1isInCDC == 1;

      //------------Second track variables----------------
      for (unsigned int l = k + 1; l < m_pionParticles->getListSize(); l++) {

        Particle* part2 = m_pionParticles->getParticle(l);
        if (!part2) continue;

        const auto chargep2 = part2->getCharge();
        if (abs(chargep2) != 1 || (chargep1 + chargep2 != 0)) continue;

        const ECLCluster* eclTrack2 = part2->getECLCluster();
        if (!eclTrack2) continue;
        if (!eclTrack2->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) continue;

        const Track* track2 = part2->getTrack();
        if (!track2) continue;

        const TrackFitResult* trackFit2 = track2->getTrackFitResultWithClosestMass(Const::pion);
        if (!trackFit2) continue;

        const ROOT::Math::PxPyPzEVector V4p2 = trackFit2->get4Momentum();
        const B2Vector3D V3p2 = (PCmsLabTransform::labToCms(V4p2)).Vect();

        const double p2MomLab = V4p2.P();
        double lowestP = p2MomLab;
        const double p2Eop = Variable::eclClusterEoP(part2);
        const double p2CDChits = trackFit2->getHitPatternCDC().getNHits();
        const PIDLikelihood* p2Pid = part2->getPIDLikelihood();
        bool p2hasKLMid = 0;
        if (p2Pid) p2hasKLMid = p2Pid->isAvailable(Const::KLM);
        const double p2isInCDC = Variable::inCDCAcceptance(part2);
        const double p2clusPhi = Variable::eclClusterPhi(part2);

        const double Pp2 = V3p2.Mag();
        const double Thetap2 = (V3p2).Theta() * TMath::RadToDeg();
        const double Phip2 = (V3p2).Phi() * TMath::RadToDeg();

        const double acopPhi = fabs(180 - fabs(Phip1 - Phip2));
        const double acopTheta = fabs(fabs(Thetap1 + Thetap2) - 180);

        const double enECLTrack2 = eclTrack2->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons);

        const bool goodTrk2 = enECLTrack2 > 0 && enECLTrack2 < 0.4 && p2Eop < maxEoP && p2CDChits > 0
                              && ((p2hasKLMid == 0 && enECLTrack2 < 0.25 && p2MomLab < 2.0) || p2hasKLMid == 1) && p2isInCDC == 1;

        double eTotMumuTracks = enECLTrack1 + enECLTrack2;
        double EMumutot = eTotMumuTracks + eMumuTotGammas;

        bool mumutight_tag = enECLTrack1 < 0.5 && enECLTrack2 < 0.5 && EMumutot < 2 && acopPhi < 10 && acopTheta < 10 && nTracks == 2
                             && Pp1 > 0.5 && Pp2 > 0.5;

        if (mumutight_tag) mumutight = 1;

        if (p1MomLab < p2MomLab) {
          lowestP = highestP;
          highestP = p2MomLab;
        }

        double diffPhi = p1clusPhi - p2clusPhi;
        if (fabs(diffPhi) > M_PI) {
          if (diffPhi > M_PI) {
            diffPhi = diffPhi - 2 * M_PI;
          } else {
            diffPhi = 2 * M_PI + diffPhi;
          }
        }

        const double recoilP = fr.getMomentum(pIN - V4p1 - V4p2).P();

        const bool radmumu_tag = nTracks < 4 && goodTrk1 == 1 && goodTrk2 == 1 && highestP > 1 && lowestP < 3 && (p1hasKLMid == 1
                                 || p2hasKLMid == 1) && abs(diffPhi) >= 0.5 * M_PI && recoilP > 0.1 && (enECLTrack1 <= 0.25 || enECLTrack2 <= 0.25);

        if (radmumu_tag) radmumu = 1;

      }
    }
  }

  calculationResult["MumuTight"] = mumutight;
  calculationResult["Radmumu"] = radmumu;

  //Retrieve variables for HadronB skims
  double EsumPiHad = 0;
  double PzPiHad = 0;
  int nHadTracks = m_pionHadParticles->getListSize();
  double hadronb = 0;
  double hadronb1 = 0;
  double hadronb2 = 0;
  std::vector<ROOT::Math::XYZVector> m_pionHadv3;

  for (int nPiHad = 0; nPiHad < nHadTracks; nPiHad++) {
    Particle* parPiHad = m_pionHadParticles->getParticle(nPiHad);
    ROOT::Math::PxPyPzEVector V4PiHad = PCmsLabTransform::labToCms(parPiHad->get4Vector());
    m_pionHadv3.push_back(parPiHad->getMomentum());
    EsumPiHad += V4PiHad.E();
    PzPiHad += V4PiHad.Pz();
  }

  double visibleEnergyCMSnorm = (EsumPiHad + EsumGamma) / (BeamEnergyCMS() * 2.0);
  double EsumCMSnorm = eneclClusters / (BeamEnergyCMS() * 2.0);
  double PzTotCMSnorm = (PzPiHad + PzGamma) / (BeamEnergyCMS() * 2.0);

  bool hadronb_tag = nHadTracks >= 3 && visibleEnergyCMSnorm > 0.2 && abs(PzTotCMSnorm) < 0.5 && neclClusters > 1
                     && EsumCMSnorm > 0.1 && EsumCMSnorm < 0.8;

  if (hadronb_tag) {
    hadronb = 1;
    FoxWolfram fw(m_pionHadv3);
    fw.calculateBasicMoments();
    double R2 = fw.getR(2);
    if (R2 < 0.4) hadronb1 = 1;
    if (hadronb1 && nHadTracks >= 5) hadronb2 = 1;
  }

  calculationResult["HadronB"] = hadronb;
  calculationResult["HadronB1"] = hadronb1;
  calculationResult["HadronB2"] = hadronb2;

  // nKshort
  int nKshort = 0;
  double Kshort = 0.;
  const double KsMassLow = 0.468;
  const double KsMassHigh = 0.528;

  if (m_KsParticles.isValid()) {
    for (unsigned int i = 0; i < m_KsParticles->getListSize(); i++) {
      const Particle* mergeKsCand = m_KsParticles->getParticle(i);
      const double isKsCandGood = Variable::goodBelleKshort(mergeKsCand);
      const double KsCandMass = mergeKsCand->getMass();
      if (KsCandMass > KsMassLow && KsCandMass < KsMassHigh && isKsCandGood == 1.) nKshort++;
    }
  }

  if (nKshort != 0) Kshort = 1;

  calculationResult["Kshort"] = Kshort;

  // 4 leptons skim
  int nFourLep = 0;
  double fourLep = 0.;

  const double visibleEnergyCMS = visibleEnergyCMSnorm * BeamEnergyCMS() * 2.0;
  const unsigned int n_particles = m_pionHadParticles->getListSize();

  if (n_particles >= 2) {
    for (unsigned int i = 0; i < n_particles - 1; i++) {
      Particle* par1 = m_pionHadParticles->getParticle(i);
      for (unsigned int j = i + 1; j < n_particles; j++) {
        Particle* par2 = m_pionHadParticles->getParticle(j);
        const auto chSum = par1->getCharge() + par2->getCharge();
        const ROOT::Math::PxPyPzEVector V4p1 = par1->get4Vector();
        const ROOT::Math::PxPyPzEVector V4p2 = par2->get4Vector();
        const double opAng = V4p1.Theta() - V4p2.Theta();
        const ROOT::Math::PxPyPzEVector V4pSum = V4p1 + V4p2;
        const ROOT::Math::PxPyPzEVector V4pSumCMS = PCmsLabTransform::labToCms(V4pSum);
        const double ptCMS = V4pSumCMS.Pt();
        const double pzCMS = V4pSumCMS.Pz();
        const double mSum = V4pSum.M();

        const bool fourLepCand = chSum == 0 && (V4p1.P() > 0.4 && V4p2.P() > 0.4) && cos(opAng) > -0.997 && ptCMS < 0.15 && abs(pzCMS) < 2.5
                                 && mSum < 6;

        if (fourLepCand)  nFourLep++;
      }
    }
  }

  if (nFourLep != 0 && visibleEnergyCMS < 6) fourLep = 1;

  calculationResult["FourLep"] = fourLep;

  // nLambda
  unsigned int nLambda = 0;

  if (m_LambdaParticles.isValid()) {
    for (unsigned int i = 0; i < m_LambdaParticles->getListSize(); i++) {
      const Particle* mergeLambdaCand = m_LambdaParticles->getParticle(i);
      const double flightDist = Variable::flightDistance(mergeLambdaCand);
      const double flightDistErr = Variable::flightDistanceErr(mergeLambdaCand);
      const double flightSign = flightDist / flightDistErr;
      const Particle* protCand = mergeLambdaCand->getDaughter(0);
      const Particle* pionCand = mergeLambdaCand->getDaughter(1);
      const double protMom = protCand->getP();
      const double pionMom = pionCand->getP();
      const double asymPDaughters = (protMom - pionMom) / (protMom + pionMom);
      if (flightSign > 10 && asymPDaughters > 0.41) nLambda++;
    }
  }

  if (nLambda > 0) {
    calculationResult["Lambda"] = 1;
  } else {
    calculationResult["Lambda"] = 0;
  }

  // nDstp
  unsigned int nDstp1 = 0;
  unsigned int nDstp2 = 0;
  unsigned int nDstp3 = 0;
  unsigned int nDstp4 = 0;

  if (m_DstParticles.isValid() && (ntrk_bha >= 3 && Bhabha2Trk == 0)) {
    for (unsigned int i = 0; i < m_DstParticles->getListSize(); i++) {
      const Particle* allDstCand = m_DstParticles->getParticle(i);
      const double dstDecID = allDstCand->getExtraInfo("decayModeID");
      if (dstDecID == 1.) nDstp1++;
      if (dstDecID == 2.) nDstp2++;
      if (dstDecID == 3.) nDstp3++;
      if (dstDecID == 4.) nDstp4++;
    }
  }


  if (nDstp1 > 0) {
    calculationResult["Dstp1"] = 1;
  } else {
    calculationResult["Dstp1"] = 0;
  }

  if (nDstp2 > 0) {
    calculationResult["Dstp2"] = 1;
  } else {
    calculationResult["Dstp2"] = 0;
  }

  if (nDstp3 > 0) {
    calculationResult["Dstp3"] = 1;
  } else {
    calculationResult["Dstp3"] = 0;
  }

  if (nDstp4 > 0) {
    calculationResult["Dstp4"] = 1;
  } else {
    calculationResult["Dstp4"] = 0;
  }

  // nTracksOffIP
  calculationResult["nTracksOffIP"] = m_offIpParticles->getListSize();

  // Flag for events with Trigger B2Link information
  calculationResult["NeuroTRG"] = 0;

  StoreObjPtr<SoftwareTriggerResult> filter_result;
  if (filter_result.isValid()) {
    const std::map<std::string, int>& nonPrescaledResults = filter_result->getNonPrescaledResults();
    if (nonPrescaledResults.find(m_filterL1TrgNN) != nonPrescaledResults.end()) {
      const bool hasNN = (filter_result->getNonPrescaledResult(m_filterL1TrgNN) == SoftwareTriggerCutResult::c_accept);
      if (hasNN) calculationResult["NeuroTRG"] = 1;
    }
  }

  //Dimuon skim with invariant mass cut allowing at most one track not to be associated with ECL clusters

  double mumuHighMass = 0.;

  if (trackWithMaximumRho && trackWithSecondMaximumRho) {
    int hasClus = 0;
    double eclE1 = 0., eclE2 = 0.;

    const auto charge1 = trackWithMaximumRho->getCharge();
    const auto charge2 = trackWithSecondMaximumRho->getCharge();
    const auto chSum = charge1 + charge2;

    const ECLCluster* eclTrack1 = trackWithMaximumRho->getECLCluster();
    if (eclTrack1) {
      hasClus++;
      eclE1 = eclTrack1->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons);
    }

    const ECLCluster* eclTrack2 = trackWithSecondMaximumRho->getECLCluster();
    if (eclTrack2) {
      hasClus++;
      eclE2 = eclTrack2->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons);
    }
    const ROOT::Math::PxPyPzEVector  V4p1 = PCmsLabTransform::labToCms(trackWithMaximumRho->get4Vector());
    const ROOT::Math::PxPyPzEVector V4p2 = PCmsLabTransform::labToCms(trackWithSecondMaximumRho->get4Vector());

    const ROOT::Math::PxPyPzEVector V4pSum = V4p1 + V4p2;
    const double mSum = V4pSum.M();

    const double thetaSumCMS = (V4p1.Theta() + V4p2.Theta()) * TMath::RadToDeg();
    const double phi1CMS = V4p1.Phi() * TMath::RadToDeg();
    const double phi2CMS = V4p2.Phi() * TMath::RadToDeg();

    double diffPhi = phi1CMS - phi2CMS;
    if (fabs(diffPhi) > 180) {
      if (diffPhi > 180) {
        diffPhi = diffPhi - 2 * 180;
      } else {
        diffPhi = 2 * 180 + diffPhi;
      }
    }
    const double delThetaCMS = fabs(fabs(thetaSumCMS) - 180);
    const double delPhiCMS = fabs(180 - fabs(diffPhi));

    const bool mumuHighMassCand = chSum == 0 && (mSum > 8. && mSum < 12.) && hasClus > 0 && eclE1 <= 1
                                  && eclE2 <= 1 && delThetaCMS < 10 && delPhiCMS < 10;

    if (mumuHighMassCand)  mumuHighMass = 1;

  }

  calculationResult["MumuHighM"] = mumuHighMass;
}
