/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/modules/eclGammaGammaECollector/eclGammaGammaECollectorModule.h>

/* ECL headers. */
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLElementNumbers.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>

/* Basf2 headers. */
#include <analysis/ClusterUtility/ClusterUtils.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/gearbox/Const.h>
#include <framework/geometry/VectorUtil.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/TRGSummary.h>

/* ROOT headers. */
#include <Math/Vector3D.h>
#include <Math/Vector4D.h>
#include <TH2F.h>

using namespace std;
using namespace Belle2;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(eclGammaGammaECollector);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------

eclGammaGammaECollectorModule::eclGammaGammaECollectorModule() : CalibrationCollectorModule(),
  m_ECLExpGammaGammaE("ECLExpGammaGammaE"),
  m_ElectronicsCalib("ECLCrystalElectronics"), m_GammaGammaECalib("ECLCrystalEnergyGammaGamma")
{
  // Set module properties
  setDescription("Calibration Collector Module for ECL single crystal energy calibration using gamma gamma events");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("thetaLabMinDeg", m_thetaLabMinDeg, "miniumum photon theta in lab (degrees)", 0.);
  addParam("thetaLabMaxDeg", m_thetaLabMaxDeg, "maximum photon theta in lab (degrees)", 180.);
  addParam("minPairMass", m_minPairMass, "minimum invariant mass of the pair of photons (GeV/c^2)", 9.);
  addParam("mindPhi", m_mindPhi, "minimum delta phi between clusters (deg)", 179.);
  addParam("maxTime", m_maxTime, "maximum (time-<t>)/dt99 of photons", 1.);
  addParam("measureTrueEnergy", m_measureTrueEnergy, "use MC events to obtain expected energies", false);
  addParam("requireL1", m_requireL1, "only use events that have a level 1 trigger", true);
}



/**----------------------------------------------------------------------------------------*/
/**----------------------------------------------------------------------------------------*/
void eclGammaGammaECollectorModule::prepare()
{

  /**----------------------------------------------------------------------------------------*/
  B2INFO("eclGammaGammaECollector: Experiment = " << m_evtMetaData->getExperiment() << "  run = " << m_evtMetaData->getRun());

  /**----------------------------------------------------------------------------------------*/
  /** Create the histograms and register them in the data store */
  auto EnVsCrysID = new TH2F("EnVsCrysID", "Normalized energy for each crystal;crystal ID;E/Expected", ECLElementNumbers::c_NCrystals,
                             0, ECLElementNumbers::c_NCrystals, 140, 0, 1.4);
  registerObject<TH2F>("EnVsCrysID", EnVsCrysID);

  auto ExpEvsCrys = new TH1F("ExpEvsCrys", "Sum expected energy vs crystal ID;crystal ID;Energy (GeV)",
                             ECLElementNumbers::c_NCrystals, 0, ECLElementNumbers::c_NCrystals);
  registerObject<TH1F>("ExpEvsCrys", ExpEvsCrys);

  auto ElecCalibvsCrys = new TH1F("ElecCalibvsCrys", "Sum electronics calib const vs crystal ID;crystal ID;calibration constant",
                                  ECLElementNumbers::c_NCrystals, 0, ECLElementNumbers::c_NCrystals);
  registerObject<TH1F>("ElecCalibvsCrys", ElecCalibvsCrys);

  auto InitialCalibvsCrys = new TH1F("InitialCalibvsCrys",
                                     "Sum initial gamma gamma calib const vs crystal ID;crystal ID;calibration constant", ECLElementNumbers::c_NCrystals, 0,
                                     ECLElementNumbers::c_NCrystals);
  registerObject<TH1F>("InitialCalibvsCrys", InitialCalibvsCrys);

  auto CalibEntriesvsCrys = new TH1F("CalibEntriesvsCrys", "Entries in calib vs crys histograms;crystal ID;Entries per crystal",
                                     ECLElementNumbers::c_NCrystals,
                                     0, ECLElementNumbers::c_NCrystals);
  registerObject<TH1F>("CalibEntriesvsCrys", CalibEntriesvsCrys);

  /** Raw digit quantities for debugging purposes only */
  auto RawDigitAmpvsCrys = new TH2F("RawDigitAmpvsCrys", "Digit Amplitude vs crystal ID;crystal ID;Amplitude",
                                    ECLElementNumbers::c_NCrystals, 0, ECLElementNumbers::c_NCrystals, 200, 0,
                                    200000);
  registerObject<TH2F>("RawDigitAmpvsCrys", RawDigitAmpvsCrys);

  auto RawDigitTimevsCrys = new TH2F("RawDigitTimevsCrys", "Digit Time vs crystal ID;crystal ID;Time", ECLElementNumbers::c_NCrystals,
                                     0, ECLElementNumbers::c_NCrystals, 200, -2000,
                                     2000);
  registerObject<TH2F>("RawDigitTimevsCrys", RawDigitTimevsCrys);

  auto TimeMinusAverage = new TH1F("TimeMinusAverage", "Photon time - average / dt99;(T-T_ave)/dt99 ", 100, -10, 10);
  registerObject<TH1F>("TimeMinusAverage", TimeMinusAverage);


  //------------------------------------------------------------------------
  /** Parameters */
  B2INFO("Input parameters to eclGammaGammaECollector:");
  B2INFO("thetaLabMinDeg: " << m_thetaLabMinDeg);
  B2INFO("thetaLabMaxDeg: " << m_thetaLabMaxDeg);
  thetaLabMin = m_thetaLabMinDeg / TMath::RadToDeg();
  thetaLabMax = m_thetaLabMaxDeg / TMath::RadToDeg();
  B2INFO("minPairMass: " << m_minPairMass);
  B2INFO("mindPhi: " << m_mindPhi);
  B2INFO("maxTime: " << m_maxTime);
  B2INFO("measureTrueEnergy: " << m_measureTrueEnergy);
  B2INFO("requireL1: " << m_requireL1);

  /** Resize vectors */
  EperCrys.resize(ECLElementNumbers::c_NCrystals);

  /**----------------------------------------------------------------------------------------*/
  /** Get expected energies and calibration constants from DB. Need to call hasChanged() for later comparison */
  if (m_ECLExpGammaGammaE.hasChanged()) {ExpGammaGammaE = m_ECLExpGammaGammaE->getCalibVector();}
  if (m_ElectronicsCalib.hasChanged()) {ElectronicsCalib = m_ElectronicsCalib->getCalibVector();}
  if (m_GammaGammaECalib.hasChanged()) {GammaGammaECalib = m_GammaGammaECalib->getCalibVector();}

  /** Write out a few for quality control */
  for (int ic = 1; ic < 9000; ic += 1000) {
    B2INFO("DB constants for cellID=" << ic << ": ExpGammaGammaE = " << ExpGammaGammaE[ic - 1] << " ElectronicsCalib = " <<
           ElectronicsCalib[ic - 1]
           << " GammaGammaECalib = " << GammaGammaECalib[ic - 1]);
  }

  /** Verify that we have valid values for the payloads */
  for (int crysID = 0; crysID < ECLElementNumbers::c_NCrystals; crysID++) {
    if (ElectronicsCalib[crysID] <= 0) {B2FATAL("eclGammaGammaECollector: ElectronicsCalib = " << ElectronicsCalib[crysID] << " for crysID = " << crysID);}
    if (ExpGammaGammaE[crysID] == 0) {B2FATAL("eclGammaGammaECollector: ExpGammaGammaE = 0 for crysID = " << crysID);}
    if (GammaGammaECalib[crysID] == 0) {B2FATAL("eclGammaGammaECollector: GammaGammaECalib = 0 for crysID = " << crysID);}
  }

  /**----------------------------------------------------------------------------------------*/
  /** Required data objects. We don't need digits to find expected energies. */
  m_trackArray.isRequired();
  m_eclClusterArray.isRequired();
  if (!m_measureTrueEnergy) {m_eclDigitArray.isRequired();}

}


/**----------------------------------------------------------------------------------------*/
/**----------------------------------------------------------------------------------------*/
void eclGammaGammaECollectorModule::collect()
{

  /** Record the input database constants for the first call */
  if (storeCalib) {
    for (int crysID = 0; crysID < ECLElementNumbers::c_NCrystals; crysID++) {
      getObjectPtr<TH1F>("ExpEvsCrys")->Fill(crysID + 0.001, ExpGammaGammaE[crysID]);
      getObjectPtr<TH1F>("ElecCalibvsCrys")->Fill(crysID + 0.001, ElectronicsCalib[crysID]);
      getObjectPtr<TH1F>("InitialCalibvsCrys")->Fill(crysID + 0.001, GammaGammaECalib[crysID]);
      getObjectPtr<TH1F>("CalibEntriesvsCrys")->Fill(crysID + 0.001);
    }
    storeCalib = false;
  }

  /**----------------------------------------------------------------------------------------*/
  /** Check if DB objects have changed */
  bool newConst = false;
  if (m_ECLExpGammaGammaE.hasChanged()) {
    newConst = true;
    B2INFO("ECLExpGammaGammaE has changed, exp = " << m_evtMetaData->getExperiment() << "  run = " << m_evtMetaData->getRun());
    ExpGammaGammaE = m_ECLExpGammaGammaE->getCalibVector();
  }
  if (m_ElectronicsCalib.hasChanged()) {
    newConst = true;
    B2INFO("ECLCrystalElectronics has changed, exp = " << m_evtMetaData->getExperiment() << "  run = " << m_evtMetaData->getRun());
    ElectronicsCalib = m_ElectronicsCalib->getCalibVector();
  }
  if (m_GammaGammaECalib.hasChanged()) {
    newConst = true;
    B2INFO("ECLCrystalEnergyGammaGamma has changed, exp = " << m_evtMetaData->getExperiment() << "  run = " << m_evtMetaData->getRun());
    GammaGammaECalib = m_GammaGammaECalib->getCalibVector();
  }

  if (newConst) {
    for (int ic = 1; ic < 9000; ic += 1000) {
      B2INFO("DB constants for cellID=" << ic << ": ExpGammaGammaE = " << ExpGammaGammaE[ic - 1] << " ElectronicsCalib = " <<
             ElectronicsCalib[ic - 1]
             << " GammaGammaECalib = " << GammaGammaECalib[ic - 1]);
    }

    /** Verify that we have valid values for the starting calibrations */
    for (int crysID = 0; crysID < ECLElementNumbers::c_NCrystals; crysID++) {
      if (ElectronicsCalib[crysID] <= 0) {B2FATAL("eclGammaGammaECollector: ElectronicsCalib = " << ElectronicsCalib[crysID] << " for crysID = " << crysID);}
      if (ExpGammaGammaE[crysID] == 0) {B2FATAL("eclGammaGammaECollector: ExpGammaGammaE = 0 for crysID = " << crysID);}
      if (GammaGammaECalib[crysID] == 0) {B2FATAL("eclGammaGammaECollector: GammaGammaECalib = 0 for crysID = " << crysID);}
    }
  }

  /**----------------------------------------------------------------------------------------*/
  /** If requested, require a level 1 trigger  */
  if (m_requireL1) {
    unsigned int L1TriggerResults = m_TRGResults->getTRGSummary(0);
    if (L1TriggerResults == 0) {return;}
  }

  //------------------------------------------------------------------------
  /** Event selection. First, require zero good tracks. Use pion (211) mass hypothesis. */
  int nGoodTrk = 0;
  for (auto& track : m_trackArray) {
    const TrackFitResult* temptrackFit = track.getTrackFitResult(Const::ChargedStable(211));
    if (temptrackFit) {
      double pt = temptrackFit->getTransverseMomentum();
      double z0 = temptrackFit->getZ0();
      double d0 = temptrackFit->getD0();
      double pValue = temptrackFit->getPValue();
      int nCDChits = temptrackFit->getHitPatternCDC().getNHits();
      if (pt > minTrkpt && abs(z0) < maxZ0 && abs(d0) < maxD0 && pValue > minpValue && nCDChits >= minCDChits) {nGoodTrk++;}
    }
  }
  if (nGoodTrk > 0) {return;}

  //------------------------------------------------------------------------
  /** Find the two maximum energy photon clusters */
  const ECLCluster::EHypothesisBit usePhotons = ECLCluster::EHypothesisBit::c_nPhotons;
  int icMax[2] = { -1, -1};
  double maxClustE[2] = { -1., -1.};
  int nclust = m_eclClusterArray.getEntries();
  for (int ic = 0; ic < nclust; ic++) {
    if (m_eclClusterArray[ic]->hasHypothesis(usePhotons)) {
      double eClust = m_eclClusterArray[ic]->getEnergy(usePhotons);
      if (eClust > maxClustE[0]) {
        maxClustE[1] = maxClustE[0];
        icMax[1] = icMax[0];
        maxClustE[0] = eClust;
        icMax[0] = ic;
      } else if (eClust > maxClustE[1]) {
        maxClustE[1] = eClust;
        icMax[1] = ic;
      }
    }
  }

  //------------------------------------------------------------------------
  /** Selection criteria using the two clusters */
  /** Require that the two are in the specified angular region  */
  if (icMax[0] == -1 || icMax[1] == -1) {return;}
  double theta0 = m_eclClusterArray[icMax[0]]->getTheta();
  double theta1 = m_eclClusterArray[icMax[1]]->getTheta();
  if (theta0 < thetaLabMin || theta0 > thetaLabMax || theta1 < thetaLabMin || theta1 > thetaLabMax) {return;}

  /** And both have reasonably good times */
  double t0 = m_eclClusterArray[icMax[0]]->getTime();
  double t990 = m_eclClusterArray[icMax[0]]->getDeltaTime99();
  double t1 = m_eclClusterArray[icMax[1]]->getTime();
  double t991 = m_eclClusterArray[icMax[1]]->getDeltaTime99();
  double taverage = (t0 / (t990 * t990) + t1 / (t991 * t991)) / (1. / (t990 * t990) + 1. / (t991 * t991));
  if (abs(t0 - taverage) > t990 * m_maxTime || abs(t1 - taverage) > t991 * m_maxTime) {return;}

  /** And that their invariant mass is greater than specified value */
  ClusterUtils cUtil;
  const ROOT::Math::XYZVector clustervertex = cUtil.GetIPPosition();

  double phi0 = m_eclClusterArray[icMax[0]]->getPhi();
  ROOT::Math::XYZVector p30;
  VectorUtil::setMagThetaPhi(p30, maxClustE[0], theta0, phi0);
  const ROOT::Math::PxPyPzEVector p40 = cUtil.Get4MomentumFromCluster(m_eclClusterArray[icMax[0]], clustervertex, usePhotons);

  double phi1 = m_eclClusterArray[icMax[1]]->getPhi();
  ROOT::Math::XYZVector p31;
  VectorUtil::setMagThetaPhi(p31, maxClustE[1], theta1, phi1);
  const ROOT::Math::PxPyPzEVector p41 = cUtil.Get4MomentumFromCluster(m_eclClusterArray[icMax[1]], clustervertex, usePhotons);

  double pairmass = (p40 + p41).M();
  if (pairmass < m_minPairMass) {return;}

  /** And that they are back-to-back in phi */
  PCmsLabTransform boostrotate;
  ROOT::Math::PxPyPzEVector p40COM = boostrotate.rotateLabToCms() * p40;
  ROOT::Math::PxPyPzEVector p41COM = boostrotate.rotateLabToCms() * p41;
  double dphi = abs(p41COM.Phi() - p40COM.Phi()) * TMath::RadToDeg();
  if (dphi > 180.) {dphi = 360. - dphi;}
  if (dphi < m_mindPhi) {return;}

  //------------------------------------------------------------------------
  //** Find the most energetic crystal in each photon cluster */
  int crysIDMax[2] = { -1, -1};
  for (int ic = 0; ic < 2; ic++) {
    crysIDMax[ic] = m_eclClusterArray[icMax[ic]]->getMaxECellId() - 1;
  }

  //------------------------------------------------------------------------
  /** Record ECL digit amplitude as a function of CrysID when calibrating */
  memset(&EperCrys[0], 0, EperCrys.size()*sizeof EperCrys[0]);
  if (!m_measureTrueEnergy) {
    for (auto& eclDigit : m_eclDigitArray) {
      int crysID = eclDigit.getCellId() - 1;
      getObjectPtr<TH2F>("RawDigitAmpvsCrys")->Fill(crysID + 0.001, eclDigit.getAmp());

      /** GammaGammaECalib is negative if the previous iteration of the algorithm was unable to calculate a value. In this case, the input value has been stored with a minus sign */
      EperCrys[crysID] = eclDigit.getAmp() * abs(GammaGammaECalib[crysID]) * ElectronicsCalib[crysID];
      if (EperCrys[crysID] > 0.01) {
        getObjectPtr<TH2F>("RawDigitTimevsCrys")->Fill(crysID + 0.001, eclDigit.getTimeFit());
      }
    }

    /** Expected energies, get the energy of the most energetic crystal from the cluster */
  } else {

    //..getEnergyHighestCrystal() includes the leakage correction; we want raw energy.
    for (int ic = 0; ic < 2; ic++) {
      float undoCorrection = m_eclClusterArray[icMax[ic]]->getEnergyRaw() / m_eclClusterArray[icMax[ic]]->getEnergy(
                               ECLCluster::EHypothesisBit::c_nPhotons);
      EperCrys[crysIDMax[ic]] = undoCorrection * m_eclClusterArray[icMax[ic]]->getEnergyHighestCrystal();

    }
  }

  //------------------------------------------------------------------------
  //** Store the normalized energies of the two crystals */
  for (int ic = 0; ic < 2; ic++) {
    if (crysIDMax[ic] >= 0) {
      /** ExpGammaGammaE is negative if the algorithm was unable to calculate a value. In this case, the nominal input value has been stored with a minus sign */
      getObjectPtr<TH2F>("EnVsCrysID")->Fill(crysIDMax[ic] + 0.001, EperCrys[crysIDMax[ic]] / abs(ExpGammaGammaE[crysIDMax[ic]]));
      getObjectPtr<TH1F>("ExpEvsCrys")->Fill(crysIDMax[ic] + 0.001, ExpGammaGammaE[crysIDMax[ic]]);
      getObjectPtr<TH1F>("ElecCalibvsCrys")->Fill(crysIDMax[ic] + 0.001, ElectronicsCalib[crysIDMax[ic]]);
      getObjectPtr<TH1F>("InitialCalibvsCrys")->Fill(crysIDMax[ic] + 0.001, GammaGammaECalib[crysIDMax[ic]]);
      getObjectPtr<TH1F>("CalibEntriesvsCrys")->Fill(crysIDMax[ic] + 0.001);
    }
  }
  if (crysIDMax[0] >= 0 && crysIDMax[1] >= 0) {
    getObjectPtr<TH1F>("TimeMinusAverage")->Fill((t0 - taverage) / t990);
    getObjectPtr<TH1F>("TimeMinusAverage")->Fill((t1 - taverage) / t991);
  }
}
