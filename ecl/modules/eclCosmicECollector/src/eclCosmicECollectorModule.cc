/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christopher Hearty                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclCosmicECollector/eclCosmicECollectorModule.h>
#include <framework/dataobjects/EventMetaData.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <mdst/dataobjects/MCParticle.h>
#include <trg/ecl/TrgEclMapping.h>
#include <TH2F.h>

using namespace std;
using namespace Belle2;
using namespace ECL;


/**-----------------------------------------------------------------*/
//                 Register the Module
/**-----------------------------------------------------------------*/
REG_MODULE(eclCosmicECollector)

/**-----------------------------------------------------------------*/
//                 Implementation
/**-----------------------------------------------------------------*/

/**----------------------------------------------------------------------------------------*/

eclCosmicECollectorModule::eclCosmicECollectorModule() : CalibrationCollectorModule(), m_ECLExpCosmicESame("ECLExpCosmicESame"),
  m_ECLExpCosmicEDifferent("ECLExpCosmicEDifferent"), m_ElectronicsCalib("ECLCrystalElectronics"),
  m_CosmicECalib("ECLCrystalEnergyCosmic") , m_ElectronicsTime("ECLCrystalElectronicsTime"), m_TimeOffset("ECLCrystalTimeOffset")
{
  /** Set module properties */
  setDescription("Calibration Collector Module for ECL single crystal energy calibration using cosmic rays");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("minCrysE", m_minCrysE, "Minimum energy in GeV for a crystal to be considered hit", 0.010);
  addParam("mockupL1", m_mockupL1, "Calculate energy per trigger cell in lieu of trigger simulation", false);
  addParam("trigThreshold", m_trigThreshold, "Minimum energy in GeV per trigger cell to mock up L1 trigger", 0.1);
}


/**----------------------------------------------------------------------------------------*/
/**----------------------------------------------------------------------------------------*/
void eclCosmicECollectorModule::prepare()
{

  /** MetaData */
  StoreObjPtr<EventMetaData> evtMetaData;
  B2INFO("eclCosmicECollector: Experiment = " << evtMetaData->getExperiment() << "  run = " << evtMetaData->getRun());

  /** Required data objects */
  eclDigitArray.isRequired();

  /**----------------------------------------------------------------------------------------*/
  /** Create the histograms and register them in the data store */
  auto EnvsCrysSameRing = new TH2F("EnvsCrysSameRing", "Normalized energy vs crystal ID, same theta ring;crystal ID;E/Expected", 8736,
                                   0, 8736, 99, .025, 2.5);
  registerObject<TH2F>("EnvsCrysSameRing", EnvsCrysSameRing);

  auto EnvsCrysDifferentRing = new TH2F("EnvsCrysDifferentRing",
                                        "Normalized energy vs crystal ID, different theta rings;crystal ID;E/Expected", 8736, 0, 8736, 99, .025, 2.5);
  registerObject<TH2F>("EnvsCrysDifferentRing", EnvsCrysDifferentRing);

  auto ExpEvsCrysSameRing = new TH1F("ExpEvsCrysSameRing",
                                     "Sum expected energy vs crystal ID, same theta ring;crystal ID;Energy (GeV)", 8736, 0, 8736);
  registerObject<TH1F>("ExpEvsCrysSameRing", ExpEvsCrysSameRing);

  auto ExpEvsCrysDifferentRing = new TH1F("ExpEvsCrysDifferentRing",
                                          "Sum expected energy vs crystal ID, different theta rings;crystal ID;Energy (GeV)", 8736, 0, 8736);
  registerObject<TH1F>("ExpEvsCrysDifferentRing", ExpEvsCrysDifferentRing);

  auto ElecCalibvsCrysSameRing = new TH1F("ElecCalibvsCrysSameRing",
                                          "Sum electronics calib const vs crystal ID, same theta ring;crystal ID", 8736, 0, 8736);
  registerObject<TH1F>("ElecCalibvsCrysSameRing", ElecCalibvsCrysSameRing);

  auto ElecCalibvsCrysDifferentRing = new TH1F("ElecCalibvsCrysDifferentRing",
                                               "Sum electronics calib const vs crystal ID, different theta rings;crystal ID", 8736, 0, 8736);
  registerObject<TH1F>("ElecCalibvsCrysDifferentRing", ElecCalibvsCrysDifferentRing);

  auto InitialCalibvsCrysSameRing = new TH1F("InitialCalibvsCrysSameRing",
                                             "Sum initial cosmic calib const vs crystal ID, same theta ring;crystal ID", 8736, 0, 8736);
  registerObject<TH1F>("InitialCalibvsCrysSameRing", InitialCalibvsCrysSameRing);

  auto InitialCalibvsCrysDifferentRing = new TH1F("InitialCalibvsCrysDifferentRing",
                                                  "Sum initial cosmic calib const vs crystal ID, different theta rings;crystal ID", 8736, 0, 8736);
  registerObject<TH1F>("InitialCalibvsCrysDifferentRing", InitialCalibvsCrysDifferentRing);

  auto CalibEntriesvsCrysSameRing = new TH1F("CalibEntriesvsCrysSameRing",
                                             "Entries in calib vs crys histograms, same theta ring;crystal ID;Entries per crystal", 8736, 0, 8736);
  registerObject<TH1F>("CalibEntriesvsCrysSameRing", CalibEntriesvsCrysSameRing);

  auto CalibEntriesvsCrysDifferentRing = new TH1F("CalibEntriesvsCrysDifferentRing",
                                                  "Entries in calib vs crys histograms, different theta rings;crystal ID;Entries per crystal", 8736, 0, 8736);
  registerObject<TH1F>("CalibEntriesvsCrysDifferentRing", CalibEntriesvsCrysDifferentRing);

  auto RawDigitAmpvsCrys = new TH2F("RawDigitAmpvsCrys", "Digit Amplitude vs crystal ID;crystal ID;Amplitude", 8736, 0, 8736, 100, 0,
                                    2000);
  registerObject<TH2F>("RawDigitAmpvsCrys", RawDigitAmpvsCrys);

  auto RawDigitTimevsCrys = new TH2F("RawDigitTimevsCrys", "Shifted digit Time vs crystal ID;crystal ID;Time", 8736, 0, 8736, 200,
                                     -2000, 2000);
  registerObject<TH2F>("RawDigitTimevsCrys", RawDigitTimevsCrys);

  /**----------------------------------------------------------------------------------------*/
  /** Parameters */
  B2INFO("Input parameters to eclCosmicECollector:");
  B2INFO("trigThreshold: " << m_trigThreshold);
  B2INFO("minCrysE: " << m_minCrysE);

  /** Resize vectors */
  FirstSet.resize(8737);
  EperCrys.resize(8736);
  HitCrys.resize(8736);
  EnergyPerTC.resize(576);
  TCperCrys.resize(8736);


  /**----------------------------------------------------------------------------------------*/
  /** Get expected energies and electronics calibration constants from DB. Need to call hasChanged() for later comparison */
  if (m_ECLExpCosmicESame.hasChanged()) {ExpCosmicESame = m_ECLExpCosmicESame->getCalibVector();}
  if (m_ECLExpCosmicEDifferent.hasChanged()) {ExpCosmicEDifferent = m_ECLExpCosmicEDifferent->getCalibVector();}
  if (m_ElectronicsCalib.hasChanged()) {ElectronicsCalib = m_ElectronicsCalib->getCalibVector();}
  if (m_CosmicECalib.hasChanged()) {CosmicECalib = m_CosmicECalib->getCalibVector();}
  if (m_ElectronicsTime.hasChanged()) {ElectronicsTime = m_ElectronicsTime->getCalibVector();}
  if (m_TimeOffset.hasChanged()) {TimeOffset = m_TimeOffset->getCalibVector();}

  /** Write out a few for quality control */
  for (int ic = 1; ic < 9000; ic += 1000) {
    B2INFO("DB constants for cellID=" << ic << ": ExpCosmicESame = " << ExpCosmicESame[ic - 1] << " ExpCosmicEDifferent = " <<
           ExpCosmicEDifferent[ic - 1] << " ElectronicsCalib = " << ElectronicsCalib[ic - 1] << " CosmicECalib = " << CosmicECalib[ic - 1] <<
           " ElectronicsTime = " << ElectronicsTime[ic - 1] << " TimeOffset = " << TimeOffset[ic - 1]);
  }

  /** Verify that we have valid values for the starting calibrations */
  for (int ic = 0; ic < 8736; ic++) {
    if (ElectronicsCalib[ic] <= 0) {B2FATAL("eclCosmicECollector: ElectronicsCalib = " << ElectronicsCalib[ic] << " for crysID = " << ic);}
    if (ExpCosmicESame[ic] == 0) {B2FATAL("eclCosmicECollector: ExpCosmicESame = 0 for crysID = " << ic);}
    if (ExpCosmicEDifferent[ic] == 0) {B2FATAL("eclCosmicECollector: ExpCosmicEDifferent = 0 for crysID = " << ic);}
    if (CosmicECalib[ic] == 0) {B2FATAL("eclCosmicECollector: CosmicECalib = 0 for crysID = " << ic);}
  }

  /**----------------------------------------------------------------------------------------*/
  /** ECL geometry */

  /** Number of crystals in each theta ring */
  const short m_crystalsPerRing[69] = {
    48, 48, 64, 64, 64, 96, 96, 96, 96, 96, 96, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 96, 96, 96, 96, 96, 64, 64, 64
  };

  /** Crystal ID (i.e. cellID-1) for first crystal of each ThetaID */
  short firstCrystal[69] = {};
  for (int it = 1; it < 69; it++) {firstCrystal[it] = firstCrystal[it - 1] + m_crystalsPerRing[it - 1];}

  /** ThetaID and PhiID of each crystal */
  std::vector<short int> ThetaIDCrys(8736);
  std::vector<short int> PhiIDCrys(8736);
  int cID = 0;
  for (int it = 0; it < 69; it++) {
    for (int ic = 0; ic < m_crystalsPerRing[it]; ic++) {
      ThetaIDCrys.at(cID) = it;
      PhiIDCrys.at(cID) = ic;
      cID++;
    }
  }

  /**----------------------------------------------------------------------------------------*/
  /** Find the trigger cell (TC) number for every crystal. TC is numbered from 1-576, so subtract 1 */
  TrgEclMapping* trgecl_obj = new TrgEclMapping();
  for (int crysID = 0; crysID < 8736; crysID++) {
    TCperCrys[crysID] = trgecl_obj->getTCIdFromXtalId(crysID + 1) - 1;
  }
  /**----------------------------------------------------------------------------------------*/
  /** Create triplets of crystals, center one plus immediate neighbour on each side */

  /**----------------------------------------------------------------------------------------*/
  /** ThetaID=0 (crystalID [0,47] is a special case because there is no ring below it */

  int ThetaID = 0;
  double n2Overn0 = (double)m_crystalsPerRing[ThetaID + 2] / (double)m_crystalsPerRing[ThetaID];
  for (int phiID = 0; phiID < m_crystalsPerRing[ThetaID]; phiID++) {
    int crysID = firstCrystal[ThetaID] + phiID;

    /** Neighbours in the same ring */
    int phiIDA = phiID + 1;
    if (phiIDA == m_crystalsPerRing[ThetaID]) {phiIDA = 0;}
    int phiIDB = phiID - 1;
    if (phiIDB == -1) {phiIDB = m_crystalsPerRing[ThetaID] - 1;}
    FirstSet[crysID] = CenterCrys.size();
    CenterCrys.push_back(crysID);
    NeighbourA.push_back(firstCrystal[ThetaID] + phiIDA);
    NeighbourB.push_back(firstCrystal[ThetaID] + phiIDB);

    /** Select neighbour in ThetaID=1 (always the same phiID), and two neighbours in ThetaID=2 */
    double dphiB = n2Overn0 * phiID + 0.0001;
    phiIDB = dphiB;
    CenterCrys.push_back(crysID);
    NeighbourA.push_back(firstCrystal[ThetaID + 1] + phiID);
    NeighbourB.push_back(firstCrystal[ThetaID + 2] + phiIDB);

    phiIDB++;
    CenterCrys.push_back(crysID);
    NeighbourA.push_back(firstCrystal[ThetaID + 1] + phiID);
    NeighbourB.push_back(firstCrystal[ThetaID + 2] + phiIDB);
  }

  /**----------------------------------------------------------------------------------------*/
  /** Bulk of the ECL uses the neighbour code to find pairs of nearest neighbours. Excludes first and last ThetaID.  */

  /** Roughly four nearest neighbours, plus crystal itself. cellID starts from 1 in ECLNeighbours */
  ECLNeighbours* myNeighbours4 = new ECLNeighbours("F", 0.95);

  for (int crysID = firstCrystal[1]; crysID < firstCrystal[68]; crysID++) {
    std::vector<short int> neighbours = myNeighbours4->getNeighbours(crysID + 1);

    /** Find the two neighbours in the same Theta ring, and record neighbours in adjacent theta rings */
    int nA = -1;
    int nB = -1;
    std::vector<short int> nextThetaNeighbours;
    std::vector<short int> previousThetaNeighbours;
    for (auto& ID1 : neighbours) {
      int temp0 = ID1 - 1;
      if (temp0 != crysID && ThetaIDCrys[temp0] == ThetaIDCrys[crysID] && nA == -1) {
        nA = temp0;
      } else if (temp0 != crysID && ThetaIDCrys[temp0] == ThetaIDCrys[crysID] && nA != -1) {
        nB = temp0;
      }

      /** Neighbours in adjacent theta rings */
      if (ThetaIDCrys[temp0] == ThetaIDCrys[crysID] + 1) {nextThetaNeighbours.push_back(temp0);}
      if (ThetaIDCrys[temp0] == ThetaIDCrys[crysID] - 1) {previousThetaNeighbours.push_back(temp0);}
    }

    /** There is always a neighbour pair in the same theta ring */
    if (nA >= 0 && nB >= 0) {
      FirstSet[crysID] = CenterCrys.size();
      CenterCrys.push_back(crysID);
      NeighbourA.push_back(nA);
      NeighbourB.push_back(nB);
    } else {
      B2FATAL("No neighbour pair with the same thetaID for crysID = " << crysID);
    }

    /** Now create pairs consisting of one next thetaID and one previous thetaID */
    for (auto& IDn : nextThetaNeighbours) {
      for (auto& IDp : previousThetaNeighbours) {
        CenterCrys.push_back(crysID);
        NeighbourA.push_back(IDn);
        NeighbourB.push_back(IDp);
      }
    }
  }

  /**----------------------------------------------------------------------------------------*/
  /** ThetaID=68 (crystalID [8672,8735] is a special case because there is no ring below it */

  ThetaID = 68;
  for (int phiID = 0; phiID < m_crystalsPerRing[ThetaID]; phiID++) {
    int crysID = firstCrystal[ThetaID] + phiID;

    /** Neighbours in the same ring */
    int phiIDA = phiID + 1;
    if (phiIDA == m_crystalsPerRing[ThetaID]) {phiIDA = 0;}
    int phiIDB = phiID - 1;
    if (phiIDB == -1) {phiIDB = m_crystalsPerRing[ThetaID] - 1;}
    FirstSet[crysID] = CenterCrys.size();
    CenterCrys.push_back(crysID);
    NeighbourA.push_back(firstCrystal[ThetaID] + phiIDA);
    NeighbourB.push_back(firstCrystal[ThetaID] + phiIDB);

    /** Select neighbour in Theta rings 66 and 67. Same phiID */
    CenterCrys.push_back(crysID);
    NeighbourA.push_back(firstCrystal[ThetaID - 1] + phiID);
    NeighbourB.push_back(firstCrystal[ThetaID - 2] + phiID);
  }

  /** convenient to record the total number of triplets selected in the FirstSet vector */
  FirstSet[8736] = CenterCrys.size();
}


/**----------------------------------------------------------------------------------------*/
/**----------------------------------------------------------------------------------------*/
void eclCosmicECollectorModule::collect()
{

  /**----------------------------------------------------------------------------------------*/
  /** Record the input database constants for the first call */
  if (iEvent == 0) {
    for (int crysID = 0; crysID < 8736; crysID++) {
      getObjectPtr<TH1F>("ExpEvsCrysSameRing")->Fill(crysID + 0.001, ExpCosmicESame[crysID]);
      getObjectPtr<TH1F>("ElecCalibvsCrysSameRing")->Fill(crysID + 0.001, ElectronicsCalib[crysID]);
      getObjectPtr<TH1F>("InitialCalibvsCrysSameRing")->Fill(crysID + 0.001, CosmicECalib[crysID]);
      getObjectPtr<TH1F>("CalibEntriesvsCrysSameRing")->Fill(crysID + 0.001);

      getObjectPtr<TH1F>("ExpEvsCrysDifferentRing")->Fill(crysID + 0.001, ExpCosmicEDifferent[crysID]);
      getObjectPtr<TH1F>("ElecCalibvsCrysDifferentRing")->Fill(crysID + 0.001, ElectronicsCalib[crysID]);
      getObjectPtr<TH1F>("InitialCalibvsCrysDifferentRing")->Fill(crysID + 0.001, CosmicECalib[crysID]);
      getObjectPtr<TH1F>("CalibEntriesvsCrysDifferentRing")->Fill(crysID + 0.001);

    }
  }

  /**----------------------------------------------------------------------------------------*/
  /* Metadata; check if required DB objects have changed */
  if (iEvent % 10000 == 0) {B2INFO("eclCosmicECollector: iEvent = " << iEvent);}
  iEvent++;

  if (m_ECLExpCosmicESame.hasChanged()) {B2FATAL("eclCosmicECollector: ECLExpCosmicESame has changed");}
  if (m_ECLExpCosmicEDifferent.hasChanged()) {B2FATAL("eclCosmicECollector: ECLExpCosmicEDifferent has changed");}
  if (m_ElectronicsCalib.hasChanged()) {B2FATAL("eclCosmicECollector: ElectronicsCalib has changed");}
  if (m_CosmicECalib.hasChanged()) {
    B2DEBUG(9, "eclCosmicECollector: new values for ECLCosmicECalib");
    CosmicECalib = m_CosmicECalib->getCalibVector();
  }

  /**----------------------------------------------------------------------------------------*/
  /** Record ECL digit energy as a function of crystal ID and trigger cell */
  memset(&EperCrys[0], 0, EperCrys.size()*sizeof EperCrys[0]);
  memset(&EnergyPerTC[0], 0, EnergyPerTC.size()*sizeof EnergyPerTC[0]);


  for (auto& eclDigit : eclDigitArray) {
    int crysID = eclDigit.getCellId() - 1;

    /** CosmicECalib is negative if the previous iteration of the algorithm was unable to calculate a value. In this case, the input value has been stored with a minus sign */
    EperCrys[crysID] = eclDigit.getAmp() * abs(CosmicECalib[crysID]) * ElectronicsCalib[crysID];
    EnergyPerTC[TCperCrys[crysID]] += EperCrys[crysID];
    getObjectPtr<TH2F>("RawDigitAmpvsCrys")->Fill(crysID + 0.001, eclDigit.getAmp());
    if (EperCrys[crysID] > 0.01) {
      float shiftedTime = eclDigit.getTimeFit() - ElectronicsTime[crysID] - TimeOffset[crysID];
      getObjectPtr<TH2F>("RawDigitTimevsCrys")->Fill(crysID + 0.001, shiftedTime);
    }
  }
  for (int crysID = 0; crysID < 8736; crysID++) {HitCrys[crysID] = EperCrys[crysID] > m_minCrysE;}

  /**----------------------------------------------------------------------------------------*/
  /** If requested, require a minimum energy in a trigger cell */
  if (m_mockupL1) {
    float maxTrig = 0.;
    for (int tc = 0; tc < 576; tc++) {
      if (EnergyPerTC[tc] > maxTrig) {maxTrig = EnergyPerTC[tc];}
    }
    bool ECLtrigger = maxTrig > m_trigThreshold;
    if (!ECLtrigger) {return;}
  }

  /**----------------------------------------------------------------------------------------*/
  /** Record signal if both neighbours are hit. The first set of neighbours for a particular crystal is in the same theta ring. Signal for theta neighbours is recorded separately from the case where the neighbours are in the adjacent theta rings. */
  for (int i = 0; i < FirstSet[8736]; i++) {
    if (HitCrys[NeighbourA[i]] && HitCrys[NeighbourB[i]]) {
      int crysID = CenterCrys[i];
      if (i == FirstSet[crysID]) {

        /** ExpCosmicESame is negative if the algorithm was unable to calculate a value. In this case, the nominal input value has been stored with a minus sign */
        getObjectPtr<TH2F>("EnvsCrysSameRing")->Fill(crysID + 0.001, EperCrys[crysID] / abs(ExpCosmicESame[crysID]));
        getObjectPtr<TH1F>("ExpEvsCrysSameRing")->Fill(crysID + 0.001, ExpCosmicESame[crysID]);
        getObjectPtr<TH1F>("ElecCalibvsCrysSameRing")->Fill(crysID + 0.001, ElectronicsCalib[crysID]);
        getObjectPtr<TH1F>("InitialCalibvsCrysSameRing")->Fill(crysID + 0.001, CosmicECalib[crysID]);
        getObjectPtr<TH1F>("CalibEntriesvsCrysSameRing")->Fill(crysID + 0.001);
      } else {
        getObjectPtr<TH2F>("EnvsCrysDifferentRing")->Fill(crysID + 0.001, EperCrys[crysID] / abs(ExpCosmicEDifferent[crysID]));
        getObjectPtr<TH1F>("ExpEvsCrysDifferentRing")->Fill(crysID + 0.001, ExpCosmicEDifferent[crysID]);
        getObjectPtr<TH1F>("ElecCalibvsCrysDifferentRing")->Fill(crysID + 0.001, ElectronicsCalib[crysID]);
        getObjectPtr<TH1F>("InitialCalibvsCrysDifferentRing")->Fill(crysID + 0.001, CosmicECalib[crysID]);
        getObjectPtr<TH1F>("CalibEntriesvsCrysDifferentRing")->Fill(crysID + 0.001);
      }
    }
  }
}
