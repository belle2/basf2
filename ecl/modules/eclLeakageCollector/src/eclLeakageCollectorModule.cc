/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/modules/eclLeakageCollector/eclLeakageCollectorModule.h>

/* ECL headers. */
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/geometry/ECLLeakagePosition.h>

/* Basf2 headers. */
#include <framework/gearbox/Const.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/geometry/VectorUtil.h>
#include <mdst/dataobjects/MCParticle.h>

/* Root headers. */
#include <Math/Vector3D.h>
#include <Math/VectorUtil.h>
#include <TH2F.h>
#include <TMath.h>
#include <TTree.h>

/* C++ headers. */
#include <iostream>

using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(eclLeakageCollector);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

//-----------------------------------------------------------------
eclLeakageCollectorModule::eclLeakageCollectorModule() : CalibrationCollectorModule(),
  m_mcParticleArray("MCParticles"),
  m_evtMetaData("EventMetaData")
{
  /** Set module properties */
  setDescription("Store quantities from single photon MC used to calculated ECL energy leakage corrections");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("position_bins", m_position_bins, "number of crystal subdivisions in theta and phi", 29);
  addParam("number_energies", m_number_energies, "number of generated energies", 8);
  addParam("energies_forward", m_energies_forward, "generated photon energies, forward", std::vector<double> {0.030, 0.050, 0.100, 0.200, 0.483, 1.166, 2.816, 6.800});
  addParam("energies_barrel", m_energies_barrel, "generated photon energies, barrel", std::vector<double> {0.030, 0.050, 0.100, 0.200, 0.458, 1.049, 2.402, 5.500});
  addParam("energies_backward", m_energies_backward, "generated photon energies, backward", std::vector<double> {0.030, 0.050, 0.100, 0.200, 0.428, 0.917, 1.962, 4.200});
  addParam("showerArrayName", m_showerArrayName, "name of ECLShower data object", std::string("ECLShowers"));
}


//-----------------------------------------------------------------
void eclLeakageCollectorModule::prepare()
{
  //-----------------------------------------------------------------
  //..Parameters and other basic info
  B2INFO("eclLeakageCollector: Experiment = " << m_evtMetaData->getExperiment() << "  run = " << m_evtMetaData->getRun());

  //..Check that input parameters are consistent
  const int n_e_forward = m_energies_forward.size();
  const int n_e_barrel = m_energies_barrel.size();
  const int n_e_backward = m_energies_backward.size();
  if (n_e_forward != m_number_energies or n_e_barrel != m_number_energies or n_e_backward != m_number_energies) {
    B2FATAL("eclLeakageCollector: length of energy vectors inconsistent with parameter number_energies: " << n_e_forward << " " <<
            n_e_barrel << " " << n_e_backward << " " << m_number_energies);
  }

  //..Store generated energies as integers in MeV
  i_energies.resize(nLeakReg, std::vector<int>(m_number_energies, 0));
  for (int ie = 0; ie < m_number_energies; ie++) {
    i_energies[0][ie] = (int)(1000.*m_energies_forward[ie] + 0.001);
    i_energies[1][ie] = (int)(1000.*m_energies_barrel[ie] + 0.001);
    i_energies[2][ie] = (int)(1000.*m_energies_backward[ie] + 0.001);
  }

  //..Require all energies are different, and that there are at least two
  if (m_number_energies < 2) {
    B2FATAL("eclLeakageCollector: require at least two energy points. m_number_energies = " << m_number_energies);
  }
  for (int ie = 0; ie < m_number_energies - 1; ie++) {
    for (int ireg = 0; ireg < nLeakReg; ireg++) {
      if (i_energies[ireg][ie] == i_energies[ireg][ie + 1]) {
        B2FATAL("eclLeakageCollector: identical energies, ireg = " << ireg << ", " << i_energies[ireg][ie] << " MeV");
      }
    }
  }

  //-----------------------------------------------------------------
  //..Write out the input parameters
  B2INFO("eclLeakageCollector parameters: ");
  B2INFO("position_bins " << m_position_bins);
  B2INFO("number_energies " << m_number_energies);
  std::cout << "energies_forward ";
  for (int ie = 0; ie < m_number_energies; ie++) {std::cout << m_energies_forward[ie] << " ";}
  std::cout << std::endl;
  std::cout << "energies_barrel ";
  for (int ie = 0; ie < m_number_energies; ie++) {std::cout << m_energies_barrel[ie] << " ";}
  std::cout << std::endl;
  std::cout << "energies_backward ";
  for (int ie = 0; ie < m_number_energies; ie++) {std::cout << m_energies_backward[ie] << " ";}
  std::cout << std::endl;
  B2INFO("showerArrayName " << m_showerArrayName);

  //-----------------------------------------------------------------
  //..Define histogram to store parameters
  const int nBinX = 3 + nLeakReg * m_number_energies;
  auto inputParameters = new TH1F("inputParameters", "eclLeakageCollector job parameters", nBinX, 0, nBinX);
  registerObject<TH1F>("inputParameters", inputParameters);

  //..TTree stores required quantities for each photon
  auto tree = new TTree("single_photon_leakage", "");
  tree->Branch("cellID", &t_cellID, "cellID/I");
  tree->Branch("thetaID", &t_thetaID, "thetaID/I");
  tree->Branch("region", &t_region, "region/I");
  tree->Branch("thetaBin", &t_thetaBin, "thetaBin/I");
  tree->Branch("phiBin", &t_phiBin, "phiBin/I");
  tree->Branch("phiMech", &t_phiMech, "phiMech/I");
  tree->Branch("energyBin", &t_energyBin, "energyBin/I");
  tree->Branch("nCrys", &t_nCrys, "nCrys/I");
  tree->Branch("energyFrac", &t_energyFrac, "energyFrac/F");
  tree->Branch("origEnergyFrac", &t_origEnergyFrac, "origEnergyFrac/F");
  tree->Branch("locationError", &t_locationError, "locationError/F");
  registerObject<TTree>("tree", tree);


  //-----------------------------------------------------------------
  //..Class to find cellID and position within crystal from theta and phi
  std::cout << "creating leakagePosition object " << std::endl;
  leakagePosition = new ECLLeakagePosition();

  //-----------------------------------------------------------------
  //..Required arrays
  m_eclShowerArray.isRequired(m_showerArrayName);
  m_mcParticleArray.isRequired();
}

//-----------------------------------------------------------------
void eclLeakageCollectorModule::collect()
{

  //-----------------------------------------------------------------
  //..First time, store the job parameters
  if (storeCalib) {
    getObjectPtr<TH1F>("inputParameters")->Fill(0.01, m_position_bins);
    getObjectPtr<TH1F>("inputParameters")->Fill(1.01, m_number_energies);
    double firstBin = 2.01;
    for (int ie = 0; ie < m_number_energies; ie++) {
      getObjectPtr<TH1F>("inputParameters")->Fill(firstBin + ie, m_energies_forward[ie]);
    }
    firstBin += m_number_energies;
    for (int ie = 0; ie < m_number_energies; ie++) {
      getObjectPtr<TH1F>("inputParameters")->Fill(firstBin + ie, m_energies_barrel[ie]);
    }
    firstBin += m_number_energies;
    for (int ie = 0; ie < m_number_energies; ie++) {
      getObjectPtr<TH1F>("inputParameters")->Fill(firstBin + ie, m_energies_backward[ie]);
    }

    //..Keep track of how many times inputParameters was filled
    int lastBin = getObjectPtr<TH1F>("inputParameters")->GetNbinsX();
    getObjectPtr<TH1F>("inputParameters")->SetBinContent(lastBin, 1.);
    storeCalib = false;
  }

  //-----------------------------------------------------------------
  //..Generated MC particle (always the first entry in the list)
  double mcLabE = m_mcParticleArray[0]->getEnergy();
  ROOT::Math::XYZVector mcp3 = m_mcParticleArray[0]->getMomentum();
  ROOT::Math::XYZVector vertex = m_mcParticleArray[0]->getProductionVertex();

  //-----------------------------------------------------------------
  //..Find the shower closest to the MC true angle
  const int nShower = m_eclShowerArray.getEntries();
  double minAngle = 999.;
  int minShower = -1;
  for (int is = 0; is < nShower; is++) {

    //..Only interested in photon hypothesis showers
    if (m_eclShowerArray[is]->getHypothesisId() == ECLShower::c_nPhotons) {

      //..Make a position vector from theta, phi, and R
      ROOT::Math::XYZVector position;
      VectorUtil::setMagThetaPhi(
        position, m_eclShowerArray[is]->getR(),
        m_eclShowerArray[is]->getTheta(), m_eclShowerArray[is]->getPhi());

      //..Direction is difference between position and vertex
      ROOT::Math::XYZVector direction = ROOT::Math::XYZVector(position) - vertex;

      double angle = ROOT::Math::VectorUtil::Angle(direction, mcp3);
      if (angle < minAngle) {
        minAngle = angle;
        minShower = is;
      }
    }
  }
  if (minShower == -1) {return;}

  //-----------------------------------------------------------------
  //..Location of selected shower.
  const int maxECellId = m_eclShowerArray[minShower]->getCentralCellId();
  const float thetaLocation = m_eclShowerArray[minShower]->getTheta();
  const float phiLocation = m_eclShowerArray[minShower]->getPhi();
  std::vector<int> positionVector = leakagePosition->getLeakagePosition(maxECellId, thetaLocation, phiLocation, m_position_bins);

  //..TTree items
  t_cellID = positionVector[0];
  t_thetaID = positionVector[1];
  t_region = positionVector[2];
  t_thetaBin = positionVector[3];
  t_phiBin = positionVector[4];
  t_phiMech = positionVector[5];

  //-----------------------------------------------------------------
  //..Generated and reconstructed energy quantities

  //..Find the generated energy bin
  const int iGenEnergyMeV = (int)(1000.*mcLabE + 0.001);
  t_energyBin = -1;
  for (int ie = 0; ie < m_number_energies; ie++) {
    if (iGenEnergyMeV == i_energies[t_region][ie]) {
      t_energyBin = ie;
      break;
    }
  }

  //..Crystals used to calculate energy
  int nForEnergy = (int)(m_eclShowerArray[minShower]->getNumberOfCrystalsForEnergy() + 0.001);
  t_nCrys = std::min(nCrysMax, nForEnergy);

  //..Reconstructed (without leakage corrections), normalized to generated
  t_energyFrac = m_eclShowerArray[minShower]->getEnergyRaw() / mcLabE;

  //..Reconstructed energy after existing leakage correction, normalized to generated
  t_origEnergyFrac = m_eclShowerArray[minShower]->getEnergy() / mcLabE;

  //-----------------------------------------------------------------
  //..Distance between generated and reconstructed positions
  const double radius = m_eclShowerArray[minShower]->getR();
  ROOT::Math::XYZVector measuredLocation;
  VectorUtil::setMagThetaPhi(
    measuredLocation, radius, thetaLocation, phiLocation);
  ROOT::Math::XYZVector measuredDirection = ROOT::Math::XYZVector(measuredLocation) - vertex;
  t_locationError = radius * ROOT::Math::VectorUtil::Angle(measuredDirection, mcp3);

  //-----------------------------------------------------------------
  //..Done
  getObjectPtr<TTree>("tree")->Fill();
}
