/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christopher Hearty                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclGammaGammaECollector/eclGammaGammaECollectorModule.h>
#include <tracking/dataobjects/ExtHit.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <framework/gearbox/Const.h>
#include <framework/dataobjects/EventMetaData.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <framework/datastore/RelationVector.h>
#include <analysis/utility/PCmsLabTransform.h>

#include <TH2F.h>

using namespace std;
using namespace Belle2;
using namespace ECL;


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
  addParam("measureTrueEnergy", m_measureTrueEnergy, "use MC events to obtain expected energies", false);
  addParam("requireL1", m_requireL1, "only use events that have a level 1 trigger", true);
}

/**----------------------------------------------------------------------------------------*/
/**----------------------------------------------------------------------------------------*/
void eclGammaGammaECollectorModule::prepare()
{

  /**----------------------------------------------------------------------------------------*/
  /** MetaData */
  StoreObjPtr<EventMetaData> evtMetaData;
  B2INFO("eclGammaGammaECollector: Experiment = " << evtMetaData->getExperiment() << "  run = " << evtMetaData->getRun());

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


  //------------------------------------------------------------------------
  /** Parameters */
  B2INFO("Input parameters to eclGammaGammaECollector:");
  B2INFO("thetaLabMinDeg: " << m_thetaLabMinDeg);
  B2INFO("thetaLabMaxDeg: " << m_thetaLabMaxDeg);
  degPerRad = 180. / TMath::Pi();
  thetaLabMin = m_thetaLabMinDeg / degPerRad;
  thetaLabMax = m_thetaLabMaxDeg / degPerRad;
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

  /** Verify that we have valid values for the starting calibrations */
  for (int crysID = 0; crysID < 8736; crysID++) {
    if (ElectronicsCalib[crysID] <= 0) {B2FATAL("eclGammaGammaECollector: ElectronicsCalib = " << ElectronicsCalib[crysID] << " for crysID = " << crysID);}
    if (ExpGammaGammaE[crysID] == 0) {B2FATAL("eclGammaGammaECollector: ExpGammaGammaE = 0 for crysID = " << crysID);}
    if (GammaGammaECalib[crysID] == 0) {B2FATAL("eclGammaGammaECollector: GammaGammaECalib = 0 for crysID = " << crysID);}
  }

  /**----------------------------------------------------------------------------------------*/
  /** Required data objects */
  TrackArray.isRequired();
  eclClusterArray.isRequired();
  eclCalDigitArray.isRequired();
  eclDigitArray.isRequired();

}


/**----------------------------------------------------------------------------------------*/
/**----------------------------------------------------------------------------------------*/
void eclGammaGammaECollectorModule::collect()
{

  /** Record the input database constants for the first call */
  if (iEvent == 0) {
    for (int crysID = 0; crysID < 8736; crysID++) {
      getObjectPtr<TH1F>("ExpEvsCrys")->Fill(crysID + 0.001, ExpGammaGammaE[crysID]);
      getObjectPtr<TH1F>("ElecCalibvsCrys")->Fill(crysID + 0.001, ElectronicsCalib[crysID]);
      getObjectPtr<TH1F>("InitialCalibvsCrys")->Fill(crysID + 0.001, GammaGammaECalib[crysID]);
      getObjectPtr<TH1F>("CalibEntriesvsCrys")->Fill(crysID + 0.001);
    }
  }

  if (iEvent % 1000 == 0) {B2INFO("eclGammaGammaECollector: iEvent = " << iEvent);}
  iEvent++;

  /**----------------------------------------------------------------------------------------*/
  /** Check if DB objects have changed */
  if (m_ECLExpGammaGammaE.hasChanged()) { B2FATAL("eclGammaGammaECollector: ExpGammaGammaE has changed");}
  if (m_ElectronicsCalib.hasChanged()) {B2FATAL("eclGammaGammaECollector: ElectronicsCalib has changed");}
  if (m_GammaGammaECalib.hasChanged()) {
    B2INFO("eclGammaGammaECollector: new values for GammaGammaECalib");
    GammaGammaECalib = m_GammaGammaECalib->getCalibVector();
    for (int ic = 1; ic < 9000; ic += 1000) {
      B2INFO("Updated GammaGammaECalib for cellID=" << ic << ": GammaGammaECalib = " << GammaGammaECalib[ic - 1]);
    }
  }

  /**----------------------------------------------------------------------------------------*/
  /** If requested, require a level 1 trigger  */
  if (m_requireL1) {
    StoreObjPtr<TRGSummary> TRGResults;
    unsigned int L1TriggerResults = TRGResults->getTRGSummary(0);
    if (L1TriggerResults == 0) {return;}
  }

  //------------------------------------------------------------------------
  /** Event selection. First, require zero good tracks. Use pion (211) mass hypothesis. */
  int nGoodTrk = 0;
  for (auto& track : TrackArray) {
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
  /** Find the two maximum energy photon (hypothesis == 5) clusters */
  int icMax[2] = { -1, -1};
  double maxClustE[2] = { -1., -1.};
  int nclust = eclClusterArray.getEntries();
  for (int ic = 0; ic < nclust; ic++) {
    if (eclClusterArray[ic]->getHypothesisId() == 5) {
      double eClust = eclClusterArray[ic]->getEnergy();
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
  double theta0 = eclClusterArray[icMax[0]]->getTheta();
  double theta1 = eclClusterArray[icMax[1]]->getTheta();
  if (theta0 < thetaLabMin || theta0 > thetaLabMax || theta1 < thetaLabMin || theta1 > thetaLabMax) {return;}

  /** And both have reasonably good times */
  double t0 = eclClusterArray[icMax[0]]->getTime();
  double t990 = eclClusterArray[icMax[0]]->getDeltaTime99();
  double t1 = eclClusterArray[icMax[1]]->getTime();
  double t991 = eclClusterArray[icMax[1]]->getDeltaTime99();
  if (abs(t0) > t990 || abs(t1) > t991) {return;}

  /** And that their invariant mass is greater than specified value */
  double phi0 = eclClusterArray[icMax[0]]->getPhi();
  TVector3 p30(0., 0., maxClustE[0]);
  p30.SetTheta(theta0);
  p30.SetPhi(phi0);
  TLorentzVector p40(p30, maxClustE[0]);

  double phi1 = eclClusterArray[icMax[1]]->getPhi();
  TVector3 p31(0., 0., maxClustE[1]);
  p31.SetTheta(theta1);
  p31.SetPhi(phi1);
  TLorentzVector p41(p31, maxClustE[1]);

  double pairmass = (p40 + p41).M();
  if (pairmass < minPairMass) {return;}

  /** And that they are back-to-back in phi */
  PCmsLabTransform boostrotate;
  TLorentzVector p40COM = boostrotate.rotateLabToCms() * p40;
  TLorentzVector p41COM = boostrotate.rotateLabToCms() * p41;
  double dphi = abs(p41COM.Phi() - p40COM.Phi()) * degPerRad;
  if (dphi > 180.) {dphi = 360. - dphi;}
  double theta0COM = p40COM.Theta();
  double theta1COM = p41COM.Theta();
  double thetaMin = theta0COM;
  if (theta1COM < theta0COM) {thetaMin = theta1COM;}
  if (dphi < mindPhi) {return;}

  //------------------------------------------------------------------------
  /** Record ECL digit amplitude as a function of CrysID */
  memset(&EperCrys[0], 0, EperCrys.size()*sizeof EperCrys[0]);
  for (auto& eclDigit : eclDigitArray) {
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
    for (auto& eclCalDigit : eclCalDigitArray) {
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
    auto eclClusterRelations = eclClusterArray[icMax[imax]]->getRelationsTo<ECLCalDigit>("ECLCalDigits");
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
}
