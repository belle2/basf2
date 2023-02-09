/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/modules/eclNOptimalCollector/eclNOptimalCollectorModule.h>

/* ECL headers. */
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/geometry/ECLNeighbours.h>

/* Basf2 headers. */
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/ECLCluster.h>

/* ROOT headers. */
#include <TH2F.h>

/* C++ headers. */
#include <iostream>

using namespace std;
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(eclNOptimalCollector);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

/**----------------------------------------------------------------------------------------*/
/**----------------------------------------------------------------------------------------*/
eclNOptimalCollectorModule::eclNOptimalCollectorModule() : CalibrationCollectorModule()
{
  // Set module properties
  setDescription("Calibration collector module to find optimal number of crystal for cluster energies");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("numberEnergies", m_numberEnergies, "number of generated energies", 8);
  addParam("energiesForward", m_energiesForward, "generated photon energies, forward", std::vector<double> {0.030, 0.050, 0.100, 0.200, 0.483, 1.166, 2.816, 6.800});
  addParam("energiesBarrel", m_energiesBarrel, "generated photon energies, barrel", std::vector<double> {0.030, 0.050, 0.100, 0.200, 0.458, 1.049, 2.402, 5.500});
  addParam("energiesBackward", m_energiesBackward, "generated photon energies, backward", std::vector<double> {0.030, 0.050, 0.100, 0.200, 0.428, 0.917, 1.962, 4.200});
  addParam("digitArrayName", m_digitArrayName, "name of ECLCalDigit data object", std::string("ECLTrimmedDigits"));
  addParam("showerArrayName", m_showerArrayName, "name of ECLShower data object", std::string("ECLTrimmedShowers"));
  addParam("nGroupPerThetaID", m_nGroupPerThetaID, "groups per standard thetaID", 8);
}

/**----------------------------------------------------------------------------------------*/
/**----------------------------------------------------------------------------------------*/
void eclNOptimalCollectorModule::prepare()
{

  //--------------------------------------------------------
  //..Check that input parameters are consistent
  const int nForward = m_energiesForward.size();
  const int nBarrel = m_energiesBarrel.size();
  const int nBackward = m_energiesBackward.size();
  if (nForward != m_numberEnergies or nBarrel != m_numberEnergies or nBackward != m_numberEnergies) {
    B2FATAL("eclNOptimalCollector: length of energy vectors inconsistent with parameter numberEnergies: " << nForward << " " <<
            nBarrel << " " << nBackward << " " << m_numberEnergies);
  }

  //..Store generated energies as integers in MeV
  iEnergies.resize(nLeakReg, std::vector<int>(m_numberEnergies, 0));
  for (int ie = 0; ie < m_numberEnergies; ie++) {
    iEnergies[0][ie] = (int)(1000.*m_energiesForward[ie] + 0.5);
    iEnergies[1][ie] = (int)(1000.*m_energiesBarrel[ie] + 0.5);
    iEnergies[2][ie] = (int)(1000.*m_energiesBackward[ie] + 0.5);
  }

  //--------------------------------------------------------
  //..Required data objects
  m_eclShowerArray.isRequired(m_showerArrayName);
  m_eclClusterArray.isRequired("ECLClusters");
  m_eclCalDigitArray.isRequired(m_digitArrayName);
  m_mcParticleArray.isRequired();

  //--------------------------------------------------------
  //..Sort the crystals into groups of similar performance

  //..Record the thetaID of each cellID
  neighbours = new ECLNeighbours("N", 1);
  std::vector<int> thetaIDofCrysID;
  std::vector<int> nCrysPerRing;
  for (int thID = 0; thID < 69; thID++) {
    const int nCrys = neighbours->getCrystalsPerRing(thID);
    nCrysPerRing.push_back(nCrys);
    for (int phID = 0; phID < nCrys; phID++) {
      thetaIDofCrysID.push_back(thID);
    }
  }

  //..First crystalID of each thetaID
  std::vector<int> firstCrysIdPerRing;
  firstCrysIdPerRing.push_back(0);
  for (int thID = 1; thID < 69; thID++) {
    const int iCrysID = firstCrysIdPerRing[thID - 1] + nCrysPerRing[thID - 1];
    firstCrysIdPerRing.push_back(iCrysID);
  }

  //..Range of thetaID from first and last cellID
  const int firstThetaId = thetaIDofCrysID[iFirstCellId - 1];
  const int lastThetaId = thetaIDofCrysID[iLastCellId - 1];

  //..Nominal groups per thetaID, but some thetaID have double that
  const int specialThetaID[4] = {5, 11, 60, 65};
  int iSp = 0;
  int firstGroupOfThetaID = -m_nGroupPerThetaID;
  for (int thID = firstThetaId; thID <= lastThetaId; thID++) {
    firstGroupOfThetaID += m_nGroupPerThetaID;
    const int nCrysPerGroup = nCrysPerRing[thID] / m_nGroupPerThetaID;
    for (int phID = 0; phID < nCrysPerRing[thID]; phID++) {
      const int iCrysID = firstCrysIdPerRing[thID] + phID;
      const int iLocalGroup = phID / nCrysPerGroup;
      iGroupOfCrystal[iCrysID] = firstGroupOfThetaID + iLocalGroup;
    }

    //..Special thetaID with double the number of groups
    if (thID == specialThetaID[iSp]) {
      iSp++;
      firstGroupOfThetaID += m_nGroupPerThetaID;
      for (int phID = 1; phID < nCrysPerRing[thID]; phID += 3) {
        const int iCrysID = firstCrysIdPerRing[thID] + phID;
        const int iLocalGroup = phID / nCrysPerGroup;
        iGroupOfCrystal[iCrysID] = firstGroupOfThetaID + iLocalGroup;
      }

    }
  }

  //..Assign crystals outside of the useful range to the first or last group
  for (int ic = 1; ic < iFirstCellId; ic++) {
    const int iCrysID = ic - 1;
    iGroupOfCrystal[iCrysID] = iGroupOfCrystal[iFirstCellId - 1];
  }

  for (int ic = iLastCellId + 1; ic <= ECLElementNumbers::c_NCrystals; ic++) {
    const int iCrysID = ic - 1;
    iGroupOfCrystal[iCrysID] = iGroupOfCrystal[iLastCellId - 1];
  }

  //..Number of groups
  nCrystalGroups = iGroupOfCrystal[iLastCellId - 1] + 1;

  //-----------------------------------------------------------------
  //..Define histogram to store parameters
  const int nBinX = 3 + nLeakReg * m_numberEnergies;
  auto inputParameters = new TH1F("inputParameters", "eclNOptimalCollector job parameters", nBinX, 0, nBinX);
  registerObject<TH1F>("inputParameters", inputParameters);

  //..Store group number for each crystal
  auto groupNumberOfEachCellID = new TH1F("groupNumberOfEachCellID", "group number of each cellID;cellID",
                                          ECLElementNumbers::c_NCrystals, 1, 8737);
  registerObject<TH1F>("groupNumberOfEachCellID", groupNumberOfEachCellID);

  //-----------------------------------------------------------------
  //..Histograms to store the sum of n crystals. One per group / energy.
  //  Include an extra nCrystal bin at the end to store the current raw energy
  const int nHist = nCrystalGroups * m_numberEnergies;
  TH2F* eSum[nHist];
  TH2F* biasSum[nHist];
  int iHist = -1;
  for (int ig = 0; ig < nCrystalGroups; ig++) {
    for (int ie = 0; ie < m_numberEnergies; ie++) {
      iHist++;
      std::string name = "eSum_" + std::to_string(ig) + "_" + std::to_string(ie);
      TString hname = name;
      TString title = "energy summed over nCrys divided by eMC, group " + std::to_string(ig) + ", E point " + std::to_string(
                        ie) + ";nCrys;energy sum / Etrue";
      eSum[iHist] = new TH2F(hname, title, nCrysMax + 1, 1., nCrysMax + 2., 2000, 0., 2.);
      registerObject<TH2F>(name, eSum[iHist]);

      name = "biasSum_" + std::to_string(ig) + "_" + std::to_string(ie);
      hname = name;
      title = "energy minus mc true summing over nCrys, group " + std::to_string(ig) + ", E point " + std::to_string(
                ie) + ";nCrys;bias = energy minus mc truth (GeV)";
      biasSum[iHist] = new TH2F(hname, title, nCrysMax + 1, 1., nCrysMax + 2., 1000, -0.1, 0.1);
      registerObject<TH2F>(name, biasSum[iHist]);
    }
  }
}

/**----------------------------------------------------------------------------------------*/
/**----------------------------------------------------------------------------------------*/
void eclNOptimalCollectorModule::collect()
{

  //-----------------------------------------------------------------
  //..First time, store the job parameters
  if (storeParameters) {
    getObjectPtr<TH1F>("inputParameters")->Fill(0.01, nCrystalGroups);
    getObjectPtr<TH1F>("inputParameters")->Fill(1.01, m_numberEnergies);
    double firstBin = 2.01;
    for (int ie = 0; ie < m_numberEnergies; ie++) {
      getObjectPtr<TH1F>("inputParameters")->Fill(firstBin + ie, m_energiesForward[ie]);
    }
    firstBin += m_numberEnergies;
    for (int ie = 0; ie < m_numberEnergies; ie++) {
      getObjectPtr<TH1F>("inputParameters")->Fill(firstBin + ie, m_energiesBarrel[ie]);
    }
    firstBin += m_numberEnergies;
    for (int ie = 0; ie < m_numberEnergies; ie++) {
      getObjectPtr<TH1F>("inputParameters")->Fill(firstBin + ie, m_energiesBackward[ie]);
    }

    //..Keep track of how many times inputParameters was filled
    int lastBin = getObjectPtr<TH1F>("inputParameters")->GetNbinsX();
    getObjectPtr<TH1F>("inputParameters")->SetBinContent(lastBin, 1.);

    //..Store the group number for every cellID
    for (int ic = 1; ic < 8737; ic++) {
      getObjectPtr<TH1F>("groupNumberOfEachCellID")->Fill(ic + 0.01, iGroupOfCrystal[ic - 1]);
    }

    //..Call each eSum hist once to ensure they exist (with non-real nCrys)
    for (int ig = 0; ig < nCrystalGroups; ig++) {
      for (int ie = 0; ie < m_numberEnergies; ie++) {
        std::string histName = "eSum_" + std::to_string(ig) + "_" + std::to_string(ie);
        getObjectPtr<TH2F>(histName)->Fill(0.01, 0.96);

        std::string histNameBias = "biasSum_" + std::to_string(ig) + "_" + std::to_string(ie);
        getObjectPtr<TH2F>(histNameBias)->Fill(0.01, 0.96);
      }
    }

    storeParameters = false;
  }

  //--------------------------------------------------------
  //..Find the ECLShower (should only be one when using trimmed data object)
  int iMax = -1;
  double showerMaxE = 0.; // shower energy before leakage correction
  const int nShower = m_eclShowerArray.getEntries();
  for (int i = 0; i < nShower; i++) {
    if (m_eclShowerArray[i]->getHypothesisId() == ECLShower::c_nPhotons) {
      const double nominalE = m_eclShowerArray[i]->getEnergyRaw();
      if (nominalE > showerMaxE) {
        showerMaxE = nominalE;
        iMax = i;
      }
    }
  }

  //..Quit now if no shower
  if (iMax == -1) {return;}

  //--------------------------------------------------------
  //..Get cellID from related cluster
  int iCellId = -1;
  const auto showerClusterRelations = m_eclShowerArray[iMax]->getRelationsWith<ECLCluster>();
  const unsigned int nRelatedClusters = showerClusterRelations.size();
  if (nRelatedClusters > 0) {
    const auto cluster = showerClusterRelations.object(0);
    iCellId = cluster->getMaxECellId();
  }

  //..Quit if cellID is not in the range being calibrated
  if (iCellId<iFirstCellId or iCellId>iLastCellId) {return;}

  //..ECL region forward/barrel/backward
  int iRegion = 1;
  if (iCellId < iFirstBarrel) {
    iRegion = 0;
  } else if (iCellId > iLastBarrel) {
    iRegion = 2;
  }

  //--------------------------------------------------------
  //..Get the generated energy from the MCParticles block

  //..Should only be one entry. Quit if this is not the case.
  const int nMC = m_mcParticleArray.getEntries();
  if (nMC != 1) {return;}

  //..Energy. Convert to integer MeV to get corresponding bin
  const double eTrue = m_mcParticleArray[0]->getEnergy();
  const int iETrueMev = (int)(1000.*eTrue + 0.5);

  int iEnergy = -1;
  for (int ie = 0; ie < m_numberEnergies; ie++) {
    if (iETrueMev == iEnergies[iRegion][ie]) {
      iEnergy = ie;
      break;
    }
  }

  //..Quit if the true energy is not equal to a generated one.
  //  This happens if the cluster is reconstructed in the wrong region.
  if (iEnergy == -1) {return;}

  //--------------------------------------------------------
  //..Get the ECLCalDigits and weights associated with the cluster,
  //  plus MC true energy
  std::vector<double> digitEnergies;
  std::vector < std::pair<double, double> > energies;

  const auto showerDigitRelations = m_eclShowerArray[iMax]->getRelationsWith<ECLCalDigit>(m_digitArrayName);
  unsigned int nRelatedDigits = showerDigitRelations.size();
  for (unsigned int ir = 0; ir < nRelatedDigits; ir++) {
    const auto calDigit = showerDigitRelations.object(ir);
    const auto weight = showerDigitRelations.weight(ir);
    digitEnergies.push_back(calDigit->getEnergy() * weight);
    const double eCalDigit = weight * calDigit->getEnergy();

    //..MC energy via relation to ECLCalDigit
    auto digitMCRelations = calDigit->getRelationsTo<MCParticle>();
    double eMC = 0.;
    for (unsigned int i = 0; i < digitMCRelations.size(); i++) {
      eMC += digitMCRelations.weight(i);
    }
    std::pair<double, double> pTemp = std::make_pair(eCalDigit, eMC);
    energies.push_back(pTemp);
  }

  //..Sort digit and mc energies from lowest to highest
  std::sort(energies.begin(), energies.end());

  //--------------------------------------------------------
  //..Store the energy sum of 1, 2, ..., n crystals (max 21), and corresponding bias
  std::string histName = "eSum_" + std::to_string(iGroupOfCrystal[iCellId - 1]) + "_" + std::to_string(iEnergy);
  std::string histNameBias = "biasSum_" + std::to_string(iGroupOfCrystal[iCellId - 1]) + "_" + std::to_string(iEnergy);

  double eSumOfN = 0.;
  double biasSumOfN = 0.;
  for (int isum = 0; isum < nCrysMax; isum++) {
    int i = (int)nRelatedDigits - 1 - isum;
    if (i >= 0) {
      eSumOfN += energies[i].first;
      biasSumOfN += (energies[i].first - energies[i].second);
    }
    getObjectPtr<TH2F>(histName)->Fill(isum + 1.01, eSumOfN / eTrue);
    getObjectPtr<TH2F>(histNameBias)->Fill(isum + 1.01, biasSumOfN);
  }

  //..Also store the current raw energy for monitoring purposes
  getObjectPtr<TH2F>(histName)->Fill(nCrysMax + 1.01, showerMaxE / eTrue);

}
