/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//This module`
#include <ecl/modules/eclee5x5Collector/eclee5x5CollectorModule.h>

//Root
#include <TH2F.h>
#include <Math/Vector3D.h>
#include <Math/Vector4D.h>

//Analysis
#include <analysis/ClusterUtility/ClusterUtils.h>

//Framework
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/RelationVector.h>

//MDST
#include <mdst/dataobjects/TRGSummary.h>
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
REG_MODULE(eclee5x5Collector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------

eclee5x5CollectorModule::eclee5x5CollectorModule() : CalibrationCollectorModule(),
  m_ECLExpee5x5E("ECLExpee5x5E"), m_ElectronicsCalib("ECLCrystalElectronics"), m_ee5x5Calib("ECLCrystalEnergyee5x5"),
  m_selectdPhiData("ECLeedPhiData"), m_selectdPhiMC("ECLeedPhiMC")
{
  // Set module properties
  setDescription("Calibration Collector Module for ECL single crystal energy calibration using Bhabha events");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("thetaLabMinDeg", m_thetaLabMinDeg, "miniumum ecl cluster theta in lab (degrees)", 17.);
  addParam("thetaLabMaxDeg", m_thetaLabMaxDeg, "maximum ecl cluster theta in lab (degrees)", 150.);
  addParam("minE0", m_minE0, "minimum energy of cluster 0: E*0/sqrts", 0.45);
  addParam("minE1", m_minE1, "minimum energy of cluster 1: E*1/sqrts", 0.40);
  addParam("maxdThetaSum", m_maxdThetaSum, "maximum diff between 180 deg and sum of cluster theta* (deg)", 2.);
  addParam("dPhiScale", m_dPhiScale, "scale dPhi* cut by this factor", 1.);
  addParam("maxTime", m_maxTime, "maximum cluster time diff abs(t1-t0)/dt99", 10.);
  addParam("useCalDigits", m_useCalDigits, "use MC events to obtain expected energies", false);
  addParam("requireL1", m_requireL1, "only use events that have a level 1 trigger", false);
}



/**----------------------------------------------------------------------------------------*/
/**----------------------------------------------------------------------------------------*/
void eclee5x5CollectorModule::prepare()
{

  /**----------------------------------------------------------------------------------------*/
  m_sqrts = m_boostrotate.getCMSEnergy();
  B2INFO("eclee5x5Collector: Experiment = " << m_evtMetaData->getExperiment() << "  run = " << m_evtMetaData->getRun() << " sqrts = "
         << m_sqrts);


  /**----------------------------------------------------------------------------------------*/
  /** Create the histograms and register them in the data store */
  auto EnVsCrysID = new TH2F("EnVsCrysID", "Normalized 5x5 energy for each crystal;crystal ID;E25/Expected", 8736, 0, 8736, 200, 0.7,
                             1.2);
  registerObject<TH2F>("EnVsCrysID", EnVsCrysID);

  auto RvsCrysID = new TH1F("RvsCrysID", "E_exp x E_crysID / sigma^2;crysID;sum of E_exp x E_crysID/sigma^2", 8736, 0, 8736);
  registerObject("RvsCrysID", RvsCrysID);

  auto NRvsCrysID = new TH1F("NRvsCrysID", "Entries in RvsCrysID vs crysID;crysID;Entries in RvsCrysID", 8736, 0, 8736);
  registerObject("NRvsCrysID", NRvsCrysID);

  auto Qmatrix = new TH2F("Qmatrix", "E_i x E_j/sigma^2;crysID i;crysID j", 8736, 0, 8736, 8736, 0, 8736);
  registerObject("Qmatrix", Qmatrix);


  auto ElecCalibvsCrys = new TH1F("ElecCalibvsCrys", "Sum electronics calib const vs crystal ID;crystal ID;calibration constant",
                                  8736, 0, 8736);
  registerObject<TH1F>("ElecCalibvsCrys", ElecCalibvsCrys);
  auto ExpEvsCrys = new TH1F("ExpEvsCrys", "Sum expected energy calib const vs crystalID;crystal ID;calibration constant", 8736, 0,
                             8736);
  registerObject<TH1F>("ExpEvsCrys", ExpEvsCrys);
  auto InitialCalibvsCrys = new TH1F("InitialCalibvsCrys", "Sum initial calib const vs crystal ID;crystal ID;calibration constant",
                                     8736, 0, 8736);
  registerObject<TH1F>("InitialCalibvsCrys", InitialCalibvsCrys);

  auto CalibEntriesvsCrys = new TH1F("CalibEntriesvsCrys", "Entries in calib vs crys histograms;crystal ID;Entries per crystal", 8736,
                                     0, 8736);
  registerObject<TH1F>("CalibEntriesvsCrys", CalibEntriesvsCrys);

  auto EntriesvsCrys = new TH1F("EntriesvsCrys", "Selected Bhabha clusters vs crystal ID;crystal ID;Entries", 8736, 0, 8736);
  registerObject<TH1F>("EntriesvsCrys", EntriesvsCrys);

  auto dPhivsThetaID = new TH2F("dPhivsThetaID",
                                "Phi* vs thetaID forward, pass thetaSum,E0,E1;thetaID of forward cluster;dPhi COM (deg)", 69, 0, 69, 150, 165, 180);
  registerObject<TH2F>("dPhivsThetaID", dPhivsThetaID);

  //------------------------------------------------------------------------
  /** Parameters */
  B2INFO("Input parameters to eclee5x5Collector:");
  B2INFO("thetaLabMinDeg: " << m_thetaLabMinDeg);
  B2INFO("thetaLabMaxDeg: " << m_thetaLabMaxDeg);
  m_thetaLabMin = m_thetaLabMinDeg / TMath::RadToDeg();
  m_thetaLabMax = m_thetaLabMaxDeg / TMath::RadToDeg();
  B2INFO("minE0: " << m_minE0);
  B2INFO("minE1: " << m_minE1);
  B2INFO("maxdThetaSum: " << m_maxdThetaSum);
  B2INFO("dPhiScale: " << m_dPhiScale);
  B2INFO("maxTime: " << m_maxTime);
  B2INFO("useCalDigits: " << m_useCalDigits);
  B2INFO("requireL1: " << m_requireL1);

  /** Resize vectors */
  EperCrys.resize(8736);
  m_thetaID.resize(8736);

  /** ECL geometry */
  m_eclNeighbours5x5 = new ECL::ECLNeighbours("N", 2);

  /**----------------------------------------------------------------------------------------*/
  /** Get expected energies and calibration constants from DB. Need to call hasChanged() for later comparison */
  if (m_ECLExpee5x5E.hasChanged()) {
    Expee5x5E = m_ECLExpee5x5E->getCalibVector();
    Expee5x5Sigma = m_ECLExpee5x5E->getCalibUncVector();
  }
  if (m_ElectronicsCalib.hasChanged()) {ElectronicsCalib = m_ElectronicsCalib->getCalibVector();}
  if (m_ee5x5Calib.hasChanged()) {ee5x5Calib = m_ee5x5Calib->getCalibVector();}
  if (m_selectdPhiMC.hasChanged() and m_useCalDigits) {
    meandPhi = m_selectdPhiMC->getCalibVector();
    widthdPhi = m_selectdPhiMC->getCalibUncVector();
  } else if (m_selectdPhiData.hasChanged()) {
    meandPhi = m_selectdPhiData->getCalibVector();
    widthdPhi = m_selectdPhiData->getCalibUncVector();
  }

  /** Write out a few for quality control */
  for (int ic = 1; ic < 9000; ic += 1000) {
    B2INFO("DB constants for cellID=" << ic << ": ee5x5Calib = " << ee5x5Calib[ic - 1] << " Expee5x5E = " << Expee5x5E[ic - 1] <<
           " ElectronicsCalib = " <<
           ElectronicsCalib[ic - 1]);
  }

  /** Verify that we have valid values for the payloads */
  for (int crysID = 0; crysID < 8736; crysID++) {
    if (ElectronicsCalib[crysID] <= 0) {B2FATAL("eclee5x5Collector: ElectronicsCalib = " << ElectronicsCalib[crysID] << " for crysID = " << crysID);}
    if (Expee5x5E[crysID] == 0) {B2FATAL("eclee5x5Collector: Expee5x5E = 0 for crysID = " << crysID);}
    if (ee5x5Calib[crysID] == 0) {B2FATAL("eclee5x5Collector: ee5x5Calib = 0 for crysID = " << crysID);}
  }

  /**----------------------------------------------------------------------------------------*/
  /** Required data objects */
  m_eclClusterArray.isRequired();
  m_eclCalDigitArray.isRequired();
  m_eclDigitArray.isRequired();
  m_evtMetaData.isRequired();

  /**----------------------------------------------------------------------------------------*/
  //..Derive ThetaID of each crystal, and cut on dPhi* as a function of thetaID
  int crysID = 0;
  for (int it = 0; it < 69; it++) {

    //..dPhi* cuts are actually a function of thetaID, not crysID
    m_dPhiMin.push_back(meandPhi[crysID] - m_dPhiScale * widthdPhi[crysID]);
    m_dPhiMax.push_back(meandPhi[crysID] + m_dPhiScale * widthdPhi[crysID]);
    for (int ic = 0; ic < m_eclNeighbours5x5->getCrystalsPerRing(it); ic++) {
      m_thetaID.at(crysID) = it;
      crysID++;
    }
  }

}


/**----------------------------------------------------------------------------------------*/
/**----------------------------------------------------------------------------------------*/
void eclee5x5CollectorModule::collect()
{

  /** Record the input database constants for the first call */
  if (storeCalib) {
    for (int crysID = 0; crysID < 8736; crysID++) {
      getObjectPtr<TH1F>("ExpEvsCrys")->Fill(crysID + 0.001, Expee5x5E[crysID]);
      getObjectPtr<TH1F>("ElecCalibvsCrys")->Fill(crysID + 0.001, ElectronicsCalib[crysID]);
      getObjectPtr<TH1F>("InitialCalibvsCrys")->Fill(crysID + 0.001, ee5x5Calib[crysID]);
      getObjectPtr<TH1F>("CalibEntriesvsCrys")->Fill(crysID + 0.001);
    }
    storeCalib = false;
  }

  /**----------------------------------------------------------------------------------------*/
  /** Check if DB objects have changed */
  bool newConst = false;
  if (m_ECLExpee5x5E.hasChanged()) {
    newConst = true;
    B2INFO("ECLExpee5x5E has changed, exp = " << m_evtMetaData->getExperiment() << "  run = " << m_evtMetaData->getRun());
    Expee5x5E = m_ECLExpee5x5E->getCalibVector();
    Expee5x5Sigma = m_ECLExpee5x5E->getCalibUncVector();
  }
  if (m_ElectronicsCalib.hasChanged()) {
    newConst = true;
    B2INFO("ECLCrystalElectronics has changed, exp = " << m_evtMetaData->getExperiment() << "  run = " << m_evtMetaData->getRun());
    ElectronicsCalib = m_ElectronicsCalib->getCalibVector();
  }
  if (m_ee5x5Calib.hasChanged()) {
    newConst = true;
    B2INFO("ECLCrystalEnergyee5x5 has changed, exp = " << m_evtMetaData->getExperiment() << "  run = " << m_evtMetaData->getRun());
    ee5x5Calib = m_ee5x5Calib->getCalibVector();
  }

  if (newConst) {
    for (int ic = 1; ic < 9000; ic += 1000) {
      B2INFO("DB constants for cellID=" << ic << ": ee5x5Calib = " << ee5x5Calib[ic - 1] << " Expee5x5E = " << Expee5x5E[ic - 1] <<
             " ElectronicsCalib = " <<
             ElectronicsCalib[ic - 1]);
    }

    /** Verify that we have valid values for the starting calibrations */
    for (int crysID = 0; crysID < 8736; crysID++) {
      if (ElectronicsCalib[crysID] <= 0) {B2FATAL("eclee5x5Collector: ElectronicsCalib = " << ElectronicsCalib[crysID] << " for crysID = " << crysID);}
      if (Expee5x5E[crysID] == 0) {B2FATAL("eclee5x5Collector: Expee5x5E = 0 for crysID = " << crysID);}
      if (ee5x5Calib[crysID] == 0) {B2FATAL("eclee5x5Collector: ee5x5Calib = 0 for crysID = " << crysID);}
    }
  }

  /**----------------------------------------------------------------------------------------*/
  /** If requested, require a level 1 trigger  */
  if (m_requireL1) {
    unsigned int L1TriggerResults = m_TRGResults->getTRGSummary(0);
    if (L1TriggerResults == 0) {return;}
  }

  //------------------------------------------------------------------------
  /** Find the two maximum energy clusters. Use photon hypothesis */
  int icMax[2] = { -1, -1};
  double maxClustE[2] = { -1., -1.};
  int nclust = m_eclClusterArray.getEntries();
  for (int ic = 0; ic < nclust; ic++) {
    if (m_eclClusterArray[ic]->hasHypothesis(Belle2::ECLCluster::EHypothesisBit::c_nPhotons)) {
      double eClust = m_eclClusterArray[ic]->getEnergy(Belle2::ECLCluster::EHypothesisBit::c_nPhotons);
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
  if (icMax[1] == -1) {return;}
  double theta0 = m_eclClusterArray[icMax[0]]->getTheta();
  double theta1 = m_eclClusterArray[icMax[1]]->getTheta();
  if (theta0 < m_thetaLabMin || theta0 > m_thetaLabMax || theta1 < m_thetaLabMin || theta1 > m_thetaLabMax) {return;}

  /** And both have reasonably good times */
  double t0 = m_eclClusterArray[icMax[0]]->getTime();
  double dt990 = m_eclClusterArray[icMax[0]]->getDeltaTime99();
  double t1 = m_eclClusterArray[icMax[1]]->getTime();
  double dt991 = m_eclClusterArray[icMax[1]]->getDeltaTime99();
  double dt99min = dt990;
  if (dt991 < dt990) {dt99min = dt991;}
  if (dt99min <= 0) {dt99min = 0.0001;}
  if (abs(t1 - t0) > dt99min * m_maxTime) {return;}

  //------------------------------------------------------------------------
  /** Find COM 4-vectors */
  ClusterUtils cUtil;
  const ROOT::Math::XYZVector clustervertex = cUtil.GetIPPosition();

  double phi0 = m_eclClusterArray[icMax[0]]->getPhi();
  TVector3 p30(0., 0., maxClustE[0]);
  p30.SetTheta(theta0);
  p30.SetPhi(phi0);
  const ROOT::Math::PxPyPzEVector p40 = cUtil.Get4MomentumFromCluster(m_eclClusterArray[icMax[0]], clustervertex,
                                        ECLCluster::EHypothesisBit::c_nPhotons);

  double phi1 = m_eclClusterArray[icMax[1]]->getPhi();
  TVector3 p31(0., 0., maxClustE[1]);
  p31.SetTheta(theta1);
  p31.SetPhi(phi1);
  const ROOT::Math::PxPyPzEVector p41 = cUtil.Get4MomentumFromCluster(m_eclClusterArray[icMax[1]], clustervertex,
                                        ECLCluster::EHypothesisBit::c_nPhotons);

  /** Check how back-to-back in theta* */
  ROOT::Math::PxPyPzEVector p40COM = m_boostrotate.rotateLabToCms() * p40;
  ROOT::Math::PxPyPzEVector p41COM = m_boostrotate.rotateLabToCms() * p41;
  double theta01COM = (p41COM.Theta() + p40COM.Theta()) * TMath::RadToDeg();
  if (abs(theta01COM - 180.) > m_maxdThetaSum) {return;}

  /** Apply energy cut on both clusters */
  if (p40COM.E() < m_minE0 * m_sqrts and p41COM.E() < m_minE0 * m_sqrts) {return;}
  if (p40COM.E() < m_minE1 * m_sqrts or p41COM.E() < m_minE1 * m_sqrts) {return;}


  //------------------------------------------------------------------------
  /** Find the maximum energy crystal in each of the two clusters for dPhi cut */
  int crysIDMax[2] = { -1, -1};
  double crysEMax[2] = { -1., -1.};
  for (int imax = 0; imax < 2; imax++) {
    auto eclClusterRelations = m_eclClusterArray[icMax[imax]]->getRelationsTo<ECLCalDigit>("ECLCalDigits");
    for (unsigned int ir = 0; ir < eclClusterRelations.size(); ir++) {
      const auto calDigit = eclClusterRelations.object(ir);
      int tempCrysID = calDigit->getCellId() - 1;
      float tempE = calDigit->getEnergy();
      if (tempE > crysEMax[imax]) {
        crysEMax[imax] = tempE;
        crysIDMax[imax] = tempCrysID;
      }
    }
  }

  /** Record dPhi*, then apply cut */
  double dphiCOM = abs(p41COM.Phi() - p40COM.Phi()) * TMath::RadToDeg();
  if (dphiCOM > 180.) {dphiCOM = 360. - dphiCOM;}

  int thetaIDmin = m_thetaID[crysIDMax[0]];
  if (m_thetaID[crysIDMax[1]] < m_thetaID[crysIDMax[0]]) {thetaIDmin = m_thetaID[crysIDMax[1]];}
  getObjectPtr<TH2F>("dPhivsThetaID")->Fill(thetaIDmin + 0.001, dphiCOM);
  if (dphiCOM<m_dPhiMin.at(thetaIDmin) or dphiCOM>m_dPhiMax.at(thetaIDmin)) {return;}


  //------------------------------------------------------------------------
  /** Record ECL energy as a function of CrysID, derived from ECLDigits if data, ECLCalDigit if MC */
  memset(&EperCrys[0], 0, EperCrys.size()*sizeof EperCrys[0]);

  if (m_useCalDigits) {
    for (auto& eclCalDigit : m_eclCalDigitArray) {
      int tempCrysID = eclCalDigit.getCellId() - 1;
      EperCrys[tempCrysID] = eclCalDigit.getEnergy();
    }
  } else {
    for (auto& eclDigit : m_eclDigitArray) {
      int tempCrysID = eclDigit.getCellId() - 1;
      /** ee5x5Calib is negative if the previous iteration of the algorithm was unable to calculate a value. In this case, the input value has been stored with a minus sign */
      EperCrys[tempCrysID] = eclDigit.getAmp() * abs(ee5x5Calib[tempCrysID]) * ElectronicsCalib[tempCrysID];
    }
  }

  //------------------------------------------------------------------------
  //** Quantities needed for the 5x5 calibration */
  for (int ic = 0; ic < 2; ic++) {
    int crysMax = crysIDMax[ic];
    float expE = abs(Expee5x5E[crysMax]);
    float sigmaExp = Expee5x5Sigma[crysMax];
    std::vector<short int> neighbours = m_eclNeighbours5x5->getNeighbours(crysMax + 1);

    //** Energy in 5x5, and expected energy corrected for crystals that will not be calibrated */
    double reducedExpE = expE;
    double E25 = 0.;
    for (auto& cellID : neighbours) {
      E25 += EperCrys[cellID - 1];
      if (ee5x5Calib[cellID - 1] < 0.) {
        reducedExpE -= EperCrys[cellID - 1];
      }
    }

    //** now the vector and matrix used in the calibration */
    double rexpE = reducedExpE / sigmaExp;
    for (auto& celli : neighbours) {
      if (ee5x5Calib[celli - 1] > 0.) {
        float rEi = EperCrys[celli - 1] / sigmaExp;
        getObjectPtr<TH1F>("RvsCrysID")->Fill(celli - 0.999, rexpE * rEi);
        getObjectPtr<TH1F>("NRvsCrysID")->Fill(celli - 0.999);
        for (auto& cellj : neighbours) {
          if (ee5x5Calib[cellj - 1] > 0.) {
            float rEj = EperCrys[cellj - 1] / sigmaExp;
            getObjectPtr<TH2F>("Qmatrix")->Fill(celli - 0.999, cellj - 0.999, rEi * rEj);
          }
        }
      }
    }

    //** Record normalized energy and corresponding calib values. Expee5x5E is negative if the algorithm was unable to calculate a value. In this case, the nominal input value has been stored with a minus sign */
    getObjectPtr<TH2F>("EnVsCrysID")->Fill(crysMax + 0.001, E25 / expE);
    getObjectPtr<TH1F>("EntriesvsCrys")->Fill(crysMax + 0.001);
    getObjectPtr<TH1F>("ExpEvsCrys")->Fill(crysMax + 0.001, Expee5x5E[crysMax]);
    getObjectPtr<TH1F>("ElecCalibvsCrys")->Fill(crysMax + 0.001, ElectronicsCalib[crysMax]);
    getObjectPtr<TH1F>("InitialCalibvsCrys")->Fill(crysMax + 0.001, ee5x5Calib[crysMax]);
    getObjectPtr<TH1F>("CalibEntriesvsCrys")->Fill(crysMax + 0.001);
  }
}
