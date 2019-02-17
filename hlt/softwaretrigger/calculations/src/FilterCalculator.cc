/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chris Hearty, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <hlt/softwaretrigger/calculations/FilterCalculator.h>
#include <hlt/softwaretrigger/calculations/utilities.h>

#include <TLorentzVector.h>

#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/HitPatternCDC.h>

using namespace Belle2;
using namespace SoftwareTrigger;

struct MaximumPtTrack {
  double pT = NAN;
  const Track* track = nullptr;
  double clusterEnergySumCMS = 0;
  double clusterEnergySumLab = 0;
  double pCMS = NAN;
  double pLab = NAN;
  TLorentzVector p4CMS;
  TLorentzVector p4Lab;
};

struct SelectedECLCluster {
  const ECLCluster* cluster = nullptr;
  double energyCMS = NAN;
  TLorentzVector p4CMS;
  TLorentzVector p4Lab;
  bool isTrack = false;
  double clusterTime = NAN;
};

const double flatBoundaries[10] = {0., 19., 22., 25., 30., 35., 45., 60., 90., 180.};

void FilterCalculator::requireStoreArrays()
{
  m_tracks.isRequired();
  m_eclClusters.isRequired();
}

void FilterCalculator::doCalculation(SoftwareTriggerObject& calculationResult)
{
  calculationResult["nTrkLoose"] = 0; /**< number of loose tracks */
  calculationResult["nTrkTight"] = 0; /**< number of tight tracks */
  calculationResult["ee2leg"] = 0; /**< Bhabha, 2 tracks with accompanying ECL info */
  calculationResult["nEmedium"] = 0; /**< number of clusters with E*>m_Emedium (higher threshold) */
  calculationResult["nElow"] = 0; /**< number of clusters with E*>m_Elow (lower threshold) */
  calculationResult["nEhigh"] = 0; /**< number of clusters with E*>m_Ehigh (2 GeV) */
  calculationResult["netChargeLoose"] = 0; /**< net charge of loose tracks */
  calculationResult["maximumPCMS"] = NAN; /**< maximum p* of loose tracks (GeV/c) */
  calculationResult["eexx"] = 0;
  calculationResult["ee1leg1trk"] = 0; /**< Bhabha, 2 tracks, at least 1 of which has ECL info */
  calculationResult["nEhighLowAng"] = 0; /**< number of clusters with E*>m_Ehigh, low angles */
  calculationResult["nEsingleClust"] = 0; /**< clusters with E*> m_EsinglePhoton (1 GeV) */
  calculationResult["nEsinglePhotonBarrel"] = 0; /**< neutral clusters with E*> 1 GeV in [45,115] */
  calculationResult["nEsinglePhotonEndcap"] = 0; /**< neutral clusters with E*> 1 GeV, not barrel or low */
  calculationResult["nEsingleElectronBarrel"] = 0; /**< charged clusters with E*> 1 GeV in [45,115] */
  calculationResult["nVetoClust"] = 0; /**< clusters with E>m_Emedium and |t|/dt99 < 10 */
  calculationResult["n2GeVNeutBarrel"] = 0;
  calculationResult["n2GeVNeutEndcap"] = 0;
  calculationResult["n2GeVChrg"] = 0;
  calculationResult["n2GeVPhotonBarrel"] = 0;
  calculationResult["n2GeVPhotonEndcap"] = 0;
  calculationResult["ee1leg"] = 0; /**< track + ECL info consistent with Bhabha */
  calculationResult["ee1leg1clst"] = 0; /**< Bhabha, 2 ECL clusters, one of which is charged */
  calculationResult["ee1leg1e"] = 0; /**< Bhabha, 1 track has high energy cluster, other is electron */
  calculationResult["ee2clst"] = 0; /**< Bhabha, 2 ECL clusters */
  calculationResult["gg2clst"] = 0; /**< gg veto, 2 ECL clusters */
  calculationResult["eeee"] = 0;
  calculationResult["eemm"] = 0;
  calculationResult["eexxSelect"] = 0;
  calculationResult["radBhabha"] = 0;
  calculationResult["eeBrem"] = 0;
  calculationResult["isrRadBhabha"] = 0;
  calculationResult["muonPairECL"] = 0;
  calculationResult["selectee1leg1trk"] = 0;
  calculationResult["selectee1leg1clst"] = 0;
  calculationResult["selectee"] = 0;
  calculationResult["ggBarrelVL"] = 0;
  calculationResult["ggBarrelLoose"] = 0;
  calculationResult["ggBarrelTight"] = 0;
  calculationResult["ggEndcapVL"] = 0;
  calculationResult["ggEndcapLoose"] = 0;
  calculationResult["ggEndcapTight"] = 0;
  calculationResult["muonPairV"] = 0;
  calculationResult["selectmumu"] = 0;
  calculationResult["singleMuon"] = 0;
  calculationResult["eeFlat0"] = 0;
  calculationResult["eeFlat1"] = 0;
  calculationResult["eeFlat2"] = 0;
  calculationResult["eeFlat3"] = 0;
  calculationResult["eeFlat4"] = 0;
  calculationResult["eeFlat5"] = 0;
  calculationResult["eeFlat6"] = 0;
  calculationResult["eeFlat7"] = 0;
  calculationResult["eeFlat8"] = 0;

  calculationResult["true"] = 1;
  calculationResult["false"] = 0;

  // Some utilities
  // TODO: into constructor
  ClusterUtils cUtil;
  const TVector3 clustervertex = cUtil.GetIPPosition();
  PCmsLabTransform boostrotate;
  TLorentzVector p4ofCOM;
  p4ofCOM.SetPxPyPzE(0, 0, 0, boostrotate.getCMSEnergy());

  // Pointers to the two tracks with the maximum pt
  std::map<short, std::optional<MaximumPtTrack>> maximumPtTracks = {
    { -1, {}}, {1,  {}}
  };

  // --- Track variables -- //
  for (const Track& track : m_tracks) {
    const TrackFitResult* trackFitResult = track.getTrackFitResultWithClosestMass(Const::pion);
    if (not trackFitResult) {
      // Hu? No track fit result for pion? Ok, lets skip this track
      continue;
    }

    // TODO: Temporary cut on number of CDC hits
    if (trackFitResult->getHitPatternCDC().getNHits() == 0) {
      continue;
    }

    // Count loose and tight tracks
    const double z0 = trackFitResult->getZ0();
    if (std::abs(z0) < m_tightTrkZ0) {
      calculationResult["nTrkTight"] += 1;
    }

    // Loose tracks
    if (std::abs(z0) < m_looseTrkZ0) {
      calculationResult["nTrkLoose"] += 1;

      const short charge = trackFitResult->getChargeSign();
      calculationResult["netChargeLoose"] += charge;

      const TLorentzVector& momentumLab = trackFitResult->get4Momentum();
      const TLorentzVector momentumCMS = boostrotate.rotateLabToCms() * momentumLab;
      double pCMS = momentumCMS.Rho();
      if (std::isnan(calculationResult["maximumPCMS"]) or pCMS > calculationResult["maximumPCMS"]) {
        calculationResult["maximumPCMS"] = pCMS;
      }

      // Find the maximum pt negative [0] and positive [1] tracks
      const double pT = trackFitResult->getTransverseMomentum();
      const auto& currentMaximum = maximumPtTracks.at(charge);
      if (not currentMaximum or pT > currentMaximum->pT) {
        MaximumPtTrack newMaximum;
        newMaximum.pT = pT;
        newMaximum.track = &track;
        newMaximum.pCMS = pCMS;
        newMaximum.pLab = momentumLab.Rho();
        newMaximum.p4CMS = momentumCMS;
        newMaximum.p4Lab = momentumLab;
        maximumPtTracks[charge] = newMaximum;
      }
    }
  }

  // -- Cluster variables -- //
  std::vector<SelectedECLCluster> selectedClusters;
  for (const ECLCluster& cluster : m_eclClusters) {
    // Use the photon hypothesis, and only look at clusters with times < 200 ns
    const double time = cluster.getTime();
    if (not cluster.hasHypothesis(Belle2::ECLCluster::EHypothesisBit::c_nPhotons) or
        std::abs(time) > 200 or
        cluster.getEnergy(Belle2::ECLCluster::EHypothesisBit::c_nPhotons) < 0.1) {
      continue;
    }

    const double dt99 = cluster.getDeltaTime99();

    // Store infos if above the single photon threshold
    SelectedECLCluster selectedCluster;
    selectedCluster.p4Lab = cUtil.Get4MomentumFromCluster(&cluster, clustervertex,
                                                          Belle2::ECLCluster::EHypothesisBit::c_nPhotons);
    selectedCluster.p4CMS = boostrotate.rotateLabToCms() * selectedCluster.p4Lab; // was clustp4COM
    selectedCluster.cluster = &cluster;
    selectedCluster.clusterTime = time / dt99; // was clustT
    selectedCluster.energyCMS = selectedCluster.p4CMS.E(); // was first of ECOMPair
    selectedCluster.isTrack = cluster.isTrack(); // was tempCharge

    selectedClusters.push_back(selectedCluster);

    if (selectedCluster.energyCMS > m_E2min) {
      calculationResult["nElow"] += 1;
    }
    if (selectedCluster.energyCMS > m_E0min) {
      calculationResult["nEmedium"] += 1;
    }
    if (selectedCluster.energyCMS > m_Ehigh) {
      calculationResult["nEhigh"] += 1;
    }

    if (selectedCluster.energyCMS > m_E0min and std::abs(selectedCluster.clusterTime) < 10) {
      calculationResult["nVetoClust"] += 1;
    }

    //..Single cluster trigger objects use charge, Zernike moment, and thetaLab
    const double thetaLab = selectedCluster.p4Lab.Theta() * TMath::RadToDeg();
    const double zmva = cluster.getZernikeMVA();
    const bool photon = zmva > 0.5 and not selectedCluster.isTrack;
    const bool electron = zmva > 0.5 and selectedCluster.isTrack;

    if (selectedCluster.energyCMS > m_EsinglePhoton) {
      calculationResult["nEsingleClust"] += 1;

      const bool barrelRegion = thetaLab > 45 and thetaLab < 115;
      const bool endcapRegion = (thetaLab > 22 and thetaLab < 45) or (thetaLab > 115 and thetaLab < 145);

      if (photon and barrelRegion) {
        calculationResult["nEsinglePhotonBarrel"] += 1;
      }

      if (electron and barrelRegion) {
        calculationResult["nEsingleElectronBarrel"] += 1;
      }

      if (photon and endcapRegion) {
        calculationResult["nEsinglePhotonEndcap"] += 1;
      }
    }

    if (selectedCluster.energyCMS > m_Ehigh) {
      // TODO: different definition!
      const bool barrelRegion = thetaLab > 32 and thetaLab < 130;
      const bool endcapRegion = (thetaLab > 22 and thetaLab < 32) or (thetaLab > 130 and thetaLab < 145);
      const bool lowAngleRegion = thetaLab < 22 or thetaLab > 145;

      if (not selectedCluster.isTrack and barrelRegion) {
        calculationResult["n2GeVNeutBarrel"] += 1;
      }
      if (not selectedCluster.isTrack and endcapRegion) {
        calculationResult["n2GeVNeutEndcap"] += 1;
      }
      if (selectedCluster.isTrack and not lowAngleRegion) {
        calculationResult["n2GeVChrg"] += 1;
      }
      if (lowAngleRegion) {
        calculationResult["nEhighLowAng"] += 1;
      }
      if (photon and barrelRegion) {
        calculationResult["n2GeVPhotonBarrel"] += 1;
      }
      if (photon and endcapRegion) {
        calculationResult["n2GeVPhotonEndcap"] += 1;
      }
    }
  }
  std::sort(selectedClusters.begin(), selectedClusters.end(), [](const auto & rhs, const auto & lhs) {
    return lhs.energyCMS > rhs.energyCMS;
  });

  // -- Bhabha and two-photon lepton preparation -- //
  for (short charge : { -1, 1}) {
    auto& maximumPtTrack = maximumPtTracks.at(charge);
    if (not maximumPtTrack) {
      continue;
    }

    // Prep max two tracks for Bhabha and two-photon lepton selections
    // Track / cluster matching
    for (auto& cluster : maximumPtTrack->track->getRelationsTo<ECLCluster>()) {
      if (cluster.hasHypothesis(Belle2::ECLCluster::EHypothesisBit::c_nPhotons)) {
        maximumPtTrack->clusterEnergySumLab += cluster.getEnergy(Belle2::ECLCluster::EHypothesisBit::c_nPhotons);
      }
    }
    maximumPtTrack->clusterEnergySumCMS =
      maximumPtTrack->clusterEnergySumLab * maximumPtTrack->pCMS / maximumPtTrack->pLab;

    // Single leg Bhabha selections
    if (maximumPtTrack->clusterEnergySumCMS > 4.5) {
      calculationResult["ee1leg"] = 1;
    }

    // single muon
    if (maximumPtTrack->pCMS > 3 and maximumPtTrack->clusterEnergySumLab > 0. and maximumPtTrack->clusterEnergySumLab < 1.) {
      calculationResult["singleMuon"] = 1;
    }
  }

  // Bhabha selections using two tracks
  if (maximumPtTracks.at(-1) and maximumPtTracks.at(1)) {
    const MaximumPtTrack& negativeTrack = *maximumPtTracks.at(-1);
    const MaximumPtTrack& positiveTrack = *maximumPtTracks.at(1);

    double dphi = std::abs(negativeTrack.p4CMS.Phi() - positiveTrack.p4CMS.Phi()) * TMath::RadToDeg();
    if (dphi > 180) {
      dphi = 360 - dphi;
    }

    const double negativeClusterSum = negativeTrack.clusterEnergySumCMS;
    const double negativeClusterSumLab = negativeTrack.clusterEnergySumLab;
    const double negativeP = negativeTrack.pCMS;
    const double positiveClusterSum = positiveTrack.clusterEnergySumCMS;
    const double positiveClusterSumLab = positiveTrack.clusterEnergySumLab;
    const double positiveP = positiveTrack.pCMS;

    // two leg Bhabha
    const double thetaSum = (negativeTrack.p4CMS.Theta() + positiveTrack.p4CMS.Theta()) * TMath::RadToDeg();
    const double dthetaSum = std::abs(thetaSum - 180);
    const auto back2back = dphi > 175 and dthetaSum < 15;
    if (back2back and negativeClusterSum > 3 and positiveClusterSum > 3 and
        (negativeClusterSum > 4.5 or positiveClusterSum > 4.5)) {
      calculationResult["ee2leg"] = 1;
    }

    // one leg one track Bhabha
    if (back2back and ((negativeClusterSum > 4.5 and positiveP > 3) or (positiveClusterSum > 4.5 and negativeP > 3))) {
      calculationResult["ee1leg1trk"] = 1;
    }


    // one leg plus one electron
    if ((negativeClusterSum > 4.5 and positiveClusterSum > 0.8 * positiveP) or
        (positiveClusterSum > 4.5 and negativeClusterSum > 0.8 * negativeP)) {
      calculationResult["ee1leg1e"] = 1;
    }

    // eeee, eemm, mu mu, and radiative Bhabha selection
    const TLorentzVector p4Miss = p4ofCOM - negativeTrack.p4CMS - positiveTrack.p4CMS;
    const double pmissTheta = p4Miss.Theta() * TMath::RadToDeg();
    const double pmissp = p4Miss.Rho();
    const double relMissAngle0 = negativeTrack.p4CMS.Angle(p4Miss.Vect()) * TMath::RadToDeg();
    const double relMissAngle1 = positiveTrack.p4CMS.Angle(p4Miss.Vect()) * TMath::RadToDeg();

    const bool electronEP = positiveClusterSum > 0.8 * positiveP or negativeClusterSum > 0.8 * negativeP;
    const bool notMuonPair = negativeClusterSumLab > 1 or positiveClusterSumLab > 1;
    const double highp = std::max(negativeP, positiveP);
    const double lowp = std::min(negativeP, positiveP);
    const bool lowEdep = negativeClusterSumLab < 0.5 and positiveClusterSumLab < 0.5;


    if (calculationResult["maximumPCMS"] < 2 and dphi > 160 and (pmissTheta < 25. or pmissTheta > 155.)) {
      calculationResult["eexxSelect"] = 1;
      if (electronEP) {
        // TODO: The same??
        calculationResult["eeee"] = 1;
        calculationResult["eemm"] = 1;
      }
    }
    if (negativeP > 1. and positiveP > 1. and calculationResult["nTrkLoose"] == 2 and
        calculationResult["nTrkTight"] >= 1 and dphi < 170. and
        pmissTheta > 10. and pmissTheta < 170. and pmissp > 1. and electronEP) {
      calculationResult["radBhabha"] = 1;
    }

    if (negativeP > 2. and positiveP > 2. and calculationResult["nTrkLoose"] == 2 and
        calculationResult["nTrkTight"] >= 1 and dphi > 175. and
        pmissTheta > 10. and (pmissTheta < 5. or pmissTheta > 175.) and electronEP) {
      calculationResult["isrRadBhabha"] = 1;
    }
    if ((pmissTheta < 20. or pmissTheta > 160.) and
        ((calculationResult["maximumPCMS"] < 1.2 and dphi > 150.) or
         (calculationResult["maximumPCMS"] < 2. and dphi > 175.))) {
      calculationResult["eexx"] = 1;
    }
    if (calculationResult["nTrkLoose"] == 2 and highp > 4.5 and notMuonPair and pmissp > 1. and
        (relMissAngle0 < 5. or relMissAngle1 < 5.)) {
      calculationResult["eeBrem"] = 1;
    }

    if (calculationResult["nTrkLoose"] == 2 and highp > 4.5 and lowEdep and dphi > 175. and thetaSum > 175. and
        thetaSum < 185.) {
      calculationResult["muonPairV"] = 1;
    }
    if (highp > 3. and lowp > 2.5 and dphi > 165. and
        ((negativeClusterSumLab > 0. and negativeClusterSumLab < 1.) or
         (positiveClusterSumLab > 0. and positiveClusterSumLab < 1.))) {
      calculationResult["selectmumu"] = 1;
    }
  }

  if (selectedClusters.size() >= 2) {
    const SelectedECLCluster& firstCluster = selectedClusters[0];
    const SelectedECLCluster& secondCluster = selectedClusters[1];

    // Bhabha / gg vetoes using max two clusters
    double dphi = std::abs(firstCluster.p4CMS.Phi() - secondCluster.p4CMS.Phi()) * TMath::RadToDeg();
    if (dphi > 180) {
      dphi = 360 - dphi;
    }
    double thetaSum = (firstCluster.p4CMS.Theta() + secondCluster.p4CMS.Theta()) * TMath::RadToDeg();
    double dthetaSum = std::abs(thetaSum - 180);

    const double firstEnergy = firstCluster.p4CMS.E();
    const double secondEnergy = secondCluster.p4CMS.E();

    const bool highEnergetic = firstEnergy > 3 and secondEnergy > 3 and (firstEnergy > 4.5 or secondEnergy > 4.5);

    if (dphi > 160 and dphi < 178 and dthetaSum < 15 and highEnergetic) {
      calculationResult["ee2clst"] = 1;
    }

    if (dphi > 178 and dthetaSum < 15 and highEnergetic) {
      calculationResult["gg2clst"] = 1;
    }

    if ((calculationResult["ee2clst"] == 1 or calculationResult["gg2clst"] == 1) and calculationResult["ee1leg"] == 1) {
      calculationResult["ee1leg1clst"] = 1;
    }

    // gg and mumu accept triggers using ECL
    if (firstEnergy > 2 and secondEnergy > 2) {
      const double thetaLab0 = firstCluster.p4Lab.Theta() * TMath::RadToDeg();
      const double thetaLab1 = secondCluster.p4Lab.Theta() * TMath::RadToDeg();
      const double Elab0 = firstCluster.p4Lab.E();
      const double Elab1 = secondCluster.p4Lab.E();

      const bool barrel0 = thetaLab0 > 32. and thetaLab0 < 130.;
      const bool barrel1 = thetaLab1 > 32. and thetaLab1 < 130.;
      const bool bothClustersAbove4 = firstEnergy > 4 or secondEnergy > 4;
      const bool oneIsNeutral = not firstCluster.isTrack or not secondCluster.isTrack;
      const bool bothAreNeutral = not firstCluster.isTrack and not secondCluster.isTrack;
      const bool oneIsBarrel = barrel0 or barrel1;
      const bool dphiCutExtraLoose = dphi > 175;
      const bool dphiCutLoose = dphi > 177;
      const bool dphiCutTight = dphi > 177.5;

      if (dphiCutExtraLoose and oneIsNeutral and oneIsBarrel) {
        calculationResult["ggBarrelVL"] = 1;
      }
      if (bothClustersAbove4 and dphiCutLoose and oneIsNeutral and oneIsBarrel) {
        calculationResult["ggBarrelLoose"] = 1;
      }
      if (bothClustersAbove4 and dphiCutTight and bothAreNeutral and oneIsBarrel) {
        calculationResult["ggBarrelTight"] = 1;
      }
      if (dphiCutExtraLoose and oneIsNeutral and not oneIsBarrel) {
        calculationResult["ggEndcapVL"] = 1;
      }
      if (bothClustersAbove4 and dphiCutLoose and oneIsNeutral and not oneIsBarrel) {
        calculationResult["ggEndcapLoose"] = 1;
      }
      if (bothClustersAbove4 and dphiCutTight and bothAreNeutral and not oneIsBarrel) {
        calculationResult["ggEndcapTight"] = 1;
      }

      const double minEnergy = std::min(Elab0, Elab1);
      const double maxEnergy = std::max(Elab0, Elab1);
      // TODO: Hä? minEnergy & maxEnergy the same cut?
      if (dphi > 155 and thetaSum > 165 and thetaSum < 195 and minEnergy > 0.15 and minEnergy < 0.5 and
          maxEnergy > 0.15 and maxEnergy < 0.5) {
        calculationResult["muonPairECL"] = 1;
      }
    }

  }

  // Bhabha accept triggers.
  // Use theta_lab of negative track if available; otherwise cluster.
  double thetaFlatten = 0;

  // One leg, one cluster.
  for (short charge : { -1, 1}) {
    const auto& maximumPtTrack = maximumPtTracks.at(charge);
    if (not maximumPtTrack) {
      continue;
    }

    if (maximumPtTrack->clusterEnergySumCMS > 1.5) {
      double invMass = 0.;
      double tempFlatten = 0.;
      for (const SelectedECLCluster& cluster : selectedClusters) {
        double tempInvMass = (maximumPtTrack->p4Lab + cluster.p4Lab).M();
        if (tempInvMass > invMass) {
          invMass = tempInvMass;
          if (charge == 1) {
            tempFlatten = cluster.p4Lab.Theta() * TMath::RadToDeg();
          }
        }
      }
      if (charge == -1) {
        tempFlatten = maximumPtTrack->p4Lab.Theta() * TMath::RadToDeg();
      }
      if (invMass > 5.29) {
        calculationResult["selectee1leg1clst"] = 1;
        thetaFlatten = tempFlatten;
      }
    }
  }

  // Two legs
  if (maximumPtTracks.at(-1) and maximumPtTracks.at(1)) {
    const MaximumPtTrack& negativeTrack = *maximumPtTracks.at(-1);
    const MaximumPtTrack& positiveTrack = *maximumPtTracks.at(1);
    const double invMass = (negativeTrack.p4Lab + positiveTrack.p4Lab).M();
    if (invMass > 5.29 and (negativeTrack.clusterEnergySumCMS > 1.5 or positiveTrack.clusterEnergySumCMS > 1.5)) {
      calculationResult["selectee1leg1trk"] = 1;
    }

    thetaFlatten = negativeTrack.p4Lab.Theta() * TMath::RadToDeg();
  }

  if (calculationResult["selectee1leg1trk"] == 1 or calculationResult["selectee1leg1clst"] == 1) {
    for (int iflat = 0; iflat < 9; iflat++) {
      const std::string& eeFlatName = "eeFlat" + std::to_string(iflat);
      calculationResult[eeFlatName] =
        thetaFlatten > flatBoundaries[iflat] and thetaFlatten < flatBoundaries[iflat + 1];
      if (calculationResult[eeFlatName]) {
        calculationResult["selectee"] = 1;
      }
    }
  }
}
