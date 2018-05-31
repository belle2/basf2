/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christopher Hearty                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
//This module`
#include <ecl/modules/eclGammaGammaECollector/eclGammaGammaECollectorModule.h>

//Root
#include <TH2F.h>

//Analysis
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/ClusterUtility/ClusterUtils.h>

//Framework
#include <framework/gearbox/Const.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/RelationVector.h>

//MDST
#include <mdst/dataobjects/TRGSummary.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>

//ECL
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>



using namespace std;
using namespace Belle2;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(eclGammaGammaECollector)

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
  auto EnVsCrysID = new TH2F("EnVsCrysID", "Normalized energy for each crystal;crystal ID;E/Expected", 8736, 0, 8736, 140, 0, 1.4);
  registerObject<TH2F>("EnVsCrysID", EnVsCrysID);

  auto ExpEvsCrys = new TH1F("ExpEvsCrys", "Sum expected energy vs crystal ID;crystal ID;Energy (GeV)", 8736, 0, 8736);
  registerObject<TH1F>("ExpEvsCrys", ExpEvsCrys);

  auto ElecCalibvsCrys = new TH1F("ElecCalibvsCrys", "Sum electronics calib const vs crystal ID;crystal ID;calibration constant",
                                  8736, 0, 8736);
  registerObject<TH1F>("ElecCalibvsCrys", ElecCalibvsCrys);

  auto InitialCalibvsCrys = new TH1F("InitialCalibvsCrys",
                                     "Sum initial gamma gamma calib const vs crystal ID;crystal ID;calibration constant", 8736, 0, 8736);
  registerObject<TH1F>("InitialCalibvsCrys", InitialCalibvsCrys);

  auto CalibEntriesvsCrys = new TH1F("CalibEntriesvsCrys", "Entries in calib vs crys histograms;crystal ID;Entries per crystal", 8736,
                                     0, 8736);
  registerObject<TH1F>("CalibEntriesvsCrys", CalibEntriesvsCrys);

  /** Raw digit quantities for debugging purposes only */
  auto RawDigitAmpvsCrys = new TH2F("RawDigitAmpvsCrys", "Digit Amplitude vs crystal ID;crystal ID;Amplitude", 8736, 0, 8736, 200, 0,
                                    200000);
  registerObject<TH2F>("RawDigitAmpvsCrys", RawDigitAmpvsCrys);

  auto RawDigitTimevsCrys = new TH2F("RawDigitTimevsCrys", "Digit Time vs crystal ID;crystal ID;Time", 8736, 0, 8736, 200, -2000,
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
  B2INFO("measureTrueEnergy: " << m_measureTrueEnergy);
  B2INFO("requireL1: " << m_requireL1);

  /** Resize vectors */
  EperCrys.resize(8736);

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
  for (int crysID = 0; crysID < 8736; crysID++) {
    if (ElectronicsCalib[crysID] <= 0) {B2FATAL("eclGammaGammaECollector: ElectronicsCalib = " << ElectronicsCalib[crysID] << " for crysID = " << crysID);}
    if (ExpGammaGammaE[crysID] == 0) {B2FATAL("eclGammaGammaECollector: ExpGammaGammaE = 0 for crysID = " << crysID);}
    if (GammaGammaECalib[crysID] == 0) {B2FATAL("eclGammaGammaECollector: GammaGammaECalib = 0 for crysID = " << crysID);}
  }

  /**----------------------------------------------------------------------------------------*/
  /** Required data objects */
  m_trackArray.isRequired();
  m_eclClusterArray.isRequired();
  m_eclCalDigitArray.isRequired();
  m_eclDigitArray.isRequired();

}


/**----------------------------------------------------------------------------------------*/
/**----------------------------------------------------------------------------------------*/
void eclGammaGammaECollectorModule::collect()
{

  /** Record the input database constants for the first call */
  if (storeCalib) {
    for (int crysID = 0; crysID < 8736; crysID++) {
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
    for (int crysID = 0; crysID < 8736; crysID++) {
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
  int icMax[2] = { -1, -1};
  double maxClustE[2] = { -1., -1.};
  int nclust = m_eclClusterArray.getEntries();
  for (int ic = 0; ic < nclust; ic++) {
    if (m_eclClusterArray[ic]->getHypothesisId() == Belle2::ECLCluster::c_nPhotons) {
      double eClust = m_eclClusterArray[ic]->getEnergy();
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
  const TVector3 clustervertex = cUtil.GetIPPosition();

  double phi0 = m_eclClusterArray[icMax[0]]->getPhi();
  TVector3 p30(0., 0., maxClustE[0]);
  p30.SetTheta(theta0);
  p30.SetPhi(phi0);
  const TLorentzVector p40 = cUtil.Get4MomentumFromCluster(m_eclClusterArray[icMax[0]], clustervertex);

  double phi1 = m_eclClusterArray[icMax[1]]->getPhi();
  TVector3 p31(0., 0., maxClustE[1]);
  p31.SetTheta(theta1);
  p31.SetPhi(phi1);
  const TLorentzVector p41 = cUtil.Get4MomentumFromCluster(m_eclClusterArray[icMax[1]], clustervertex);

  double pairmass = (p40 + p41).M();
  if (pairmass < m_minPairMass) {return;}

  /** And that they are back-to-back in phi */
  PCmsLabTransform boostrotate;
  TLorentzVector p40COM = boostrotate.rotateLabToCms() * p40;
  TLorentzVector p41COM = boostrotate.rotateLabToCms() * p41;
  double dphi = abs(p41COM.Phi() - p40COM.Phi()) * TMath::RadToDeg();
  if (dphi > 180.) {dphi = 360. - dphi;}
  if (dphi < m_mindPhi) {return;}

  //------------------------------------------------------------------------
  /** Record ECL digit amplitude as a function of CrysID */
  memset(&EperCrys[0], 0, EperCrys.size()*sizeof EperCrys[0]);
  for (auto& eclDigit : m_eclDigitArray) {
    int crysID = eclDigit.getCellId() - 1;
    getObjectPtr<TH2F>("RawDigitAmpvsCrys")->Fill(crysID + 0.001, eclDigit.getAmp());

    /** GammaGammaECalib is negative if the previous iteration of the algorithm was unable to calculate a value. In this case, the input value has been stored with a minus sign */
    EperCrys[crysID] = eclDigit.getAmp() * abs(GammaGammaECalib[crysID]) * ElectronicsCalib[crysID];
    if (EperCrys[crysID] > 0.01) {
      getObjectPtr<TH2F>("RawDigitTimevsCrys")->Fill(crysID + 0.001, eclDigit.getTimeFit());
    }
  }

  /** Overwrite using ECLCalDigits if we are using these events to determine MC deposited energy */
  if (m_measureTrueEnergy) {
    for (auto& eclCalDigit : m_eclCalDigitArray) {
      int tempCrysID = eclCalDigit.getCellId() - 1;
      double tempE = eclCalDigit.getEnergy();
      EperCrys[tempCrysID] = tempE;
    }
  }

  //------------------------------------------------------------------------
  //** Find the most energetic crystal in each photon cluster */
  int crysIDMax[2] = { -1, -1};
  double crysEMax[2] = { -1., -1.};
  for (int imax = 0; imax < 2; imax++) {
    auto eclClusterRelations = m_eclClusterArray[icMax[imax]]->getRelationsTo<ECLCalDigit>("ECLCalDigits");
    for (unsigned int ir = 0; ir < eclClusterRelations.size(); ir++) {
      const auto calDigit = eclClusterRelations.object(ir);
      int tempCrysID = calDigit->getCellId() - 1;
      float tempE = EperCrys[tempCrysID];
      if (tempE > crysEMax[imax]) {
        crysEMax[imax] = tempE;
        crysIDMax[imax] = tempCrysID;
      }
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
