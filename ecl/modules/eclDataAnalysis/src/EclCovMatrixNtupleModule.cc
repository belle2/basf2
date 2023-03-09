/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/**************************************************************************
 *                                                                        *
 * Description: This module write ECL digi information in a root tuple    *
 *              to study amplitude and time info                          *
 *                                                                        *
 **************************************************************************/

/* Own header. */
#include <ecl/modules/eclDataAnalysis/EclCovMatrixNtupleModule.h>

/* ECL headers. */
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dataobjects/ECLTrig.h>
#include <ecl/geometry/ECLGeometryPar.h>

/* ROOT headers. */
#include <TFile.h>
#include <TTree.h>

using namespace std;
using namespace Belle2;
using namespace ECL;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EclCovMatrixNtuple);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
EclCovMatrixNtupleModule::EclCovMatrixNtupleModule() : Module()
{
  //Set module properties
  setDescription("EclCovMatrixNtuple: write ECL waveform and fitted time and amplitude in a root file");
  setPropertyFlags(c_ParallelProcessingCertified);
  //Parameters definition
  addParam("outputFileName", m_dataOutFileName,
           "Output root file name of this module", string("EclCovMatrixNtuple"));
  addParam("dspArrayName", m_dspArrayName, "name of input ECLDsp Array", string("ECLDsps"));
  addParam("digiArrayName", m_digiArrayName, "name of input ECLDigit Array", string("ECLDigits"));
}

void EclCovMatrixNtupleModule::initialize()
{
  m_eclDspArray.registerInDataStore(m_dspArrayName);
  m_eclDigiArray.registerInDataStore(m_digiArrayName);
  B2INFO("[EclCovMatrixNtuple Module]: Starting initialization of EclCovMatrixNtuple Module.");

  // Initializing the output root file
  string dataFileName = m_dataOutFileName + ".root";

  m_rootFile = new TFile(dataFileName.c_str(), "RECREATE");
  m_tree     = new TTree("m_tree", "EclCovMatrixNtuple tree");

  m_tree->Branch("energy",     &m_energy,     "energy/D");
  m_tree->Branch("nhits",      &m_nhits,      "nhits/I");
  m_tree->Branch("cellID",     m_cellID,      "cellID[nhits]/I");
  m_tree->Branch("theta",      m_theta,       "theta[nhits]/I");
  m_tree->Branch("phi",        m_phi,         "phi[nhits]/I");
  m_tree->Branch("hitA",       m_DspHit,      "hitA[nhits][31]/I");
  m_tree->Branch("hitT",       m_hitTime,     "hitT[nhits]/D");
  m_tree->Branch("deltaT",     m_DeltaT,      "deltaT[nhits]/D");

  m_tree->Branch("digiE",       m_hitE,       "hitE[nhits]/D");
  m_tree->Branch("digiT",      m_DigiTime,    "digiT[nhits]/I");


  B2INFO("[EclCovMatrixNtuple Module]: Finished initialising the Time Information Study in Gamma Reconstruction Module.");
}

void EclCovMatrixNtupleModule::terminate()
{
  m_rootFile->cd();
  m_tree->Write();
  m_rootFile->Close();
}

void EclCovMatrixNtupleModule::event()
{


  m_nevt++;
  m_energy = 0;



  m_nhits  = 0;

  for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++) {
    m_cellID[i]  = 0;
    m_theta[i]  = 0;
    m_phi[i]    = 0;
    m_hitTime[i] = 0;
    m_hitE[i]    = 0.0;
    m_hitTime[i] = 0.0;
    m_DigiTime[i] = 0;
    m_DeltaT[i]   = 0.0;
    for (int j = 0; j < 31; j++) {
      m_DspHit[i][j] = 0;
    }
  }


  m_nhits = m_eclDigiArray.getEntries();

  // There is only 1 ECLTrig per event
  assert(m_eclTrigArray.getEntries() == 1);
  ECLGeometryPar* eclgeo = ECLGeometryPar::Instance();
  for (const auto& adigit : m_eclDigiArray) {
    size_t cellIndex = static_cast<size_t>(adigit.getCellId() - 1);
    eclgeo->Mapping(cellIndex);
    m_theta[cellIndex] =  eclgeo->GetThetaID();
    m_phi[cellIndex] =  eclgeo->GetPhiID();
    /*
    cout << "cid : " << cellIndex
    << " theta: " << m_theta[cellIndex]
    << " phi: " << m_phi[cellIndex]
    << endl;
    */
    m_cellID[cellIndex] = cellIndex;
    m_hitE[cellIndex]     = adigit.getAmp();
    m_DigiTime[cellIndex] = adigit.getTimeFit();
    // The following DeltaT IS DIFFERENT DeltaT in igitizer by the last factor 12.!
    double deltaT = m_eclTrigArray[0]->getTimeTrig() * 508.0 / 12.0;
    m_DeltaT[cellIndex] = deltaT;
    m_hitTime[cellIndex] = 1520 - adigit.getTimeFit() - 64 * deltaT * 12.0 * 24.0 / 508.0 / 1536.0;
  }

  //The following works only because position of an ECLDigit in the ECLDigiArray
  //is the same of position of the corresponding ECLDsp in the ECLDspArray
  //Since the two a tightly related this must be enforced in a safer way
  //in the ecl data objects model (Guglielmo De Nardo)
  assert(m_eclDspArray.getEntries() == m_eclDigiArray.getEntries());
  for (const auto& eclDsp : m_eclDspArray) {
    size_t cellIndex = static_cast<size_t>(eclDsp.getCellId() - 1);
    eclDsp.getDspA(m_DspHit[cellIndex]);
  }
  m_tree->Fill();
}
