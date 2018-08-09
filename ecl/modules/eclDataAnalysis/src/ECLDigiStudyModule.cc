/*******************************************************************************
 * Module: EclCovMatrixNtuple                                                  *
 *                                                                             *
 * Contributors: Guglielmo De Nardo, Benjamin Oberhof                          *
 *                                                                             *
 * Description: This module write ECL digi information in a root tuple         *
 *              to study amplitude and time info                               *
 *                                                                             *
 * This software is provided "as is" without any warranty. Due to the          *
 * limited knowledge of BASF2 by the author some parts of the module might     *
 * be written more elegantly. Eventualy also the reconstrcution/analysis tools *
 * will be improved and parts of the code might become obsolete or could be    *
 * performed in a better way. If you find a better solution to anything below  *
 * please fill free to modify it.                                              *
 *******************************************************************************/
//This module
#include <ecl/modules/eclDataAnalysis/ECLDigiStudyModule.h>

// STL
#include <algorithm>

//ROOT
#include <TTree.h>
#include <TFile.h>

//ECL
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dataobjects/ECLTrig.h>
#include <ecl/dataobjects/ECLHit.h>
#include <ecl/geometry/ECLGeometryPar.h>



using namespace std;
using namespace Belle2;
using namespace ECL;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLDigiStudy);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
ECLDigiStudyModule::ECLDigiStudyModule() : Module()
{
  //Set module properties
  setDescription("EclCovMatrixNtuple: write ECL waveform and fitted time and amplitude in a root file");
  setPropertyFlags(c_ParallelProcessingCertified);
  //Parameters definition
  addParam("outputFileName", m_dataOutFileName,
           "Output root file name of this module", string("digistudy"));
  addParam("dspArrayName1", m_dspArrayName1, "name of input ECLDsp Array 1", string("ECLDsps"));
  addParam("digiArrayName1", m_digiArrayName1, "name of input ECLDigit Array 1", string("ECLDigits"));
  addParam("dspArrayName2", m_dspArrayName2, "name of input ECLDsp Array 2", string("ECLDspsPureCsI"));
  addParam("digiArrayName2", m_digiArrayName2, "name of input ECLDigit Array 2", string("ECLDigitsPureCsI"));
}

void ECLDigiStudyModule::initialize()
{

  m_eclDspArray1.registerInDataStore(m_dspArrayName1);
  m_eclDspArray2.registerInDataStore(m_dspArrayName2);
  m_eclDigiArray1.registerInDataStore(m_digiArrayName1);
  m_eclDigiArray2.registerInDataStore(m_digiArrayName2);

  B2INFO("[EclCovMatrixNtuple Module]: Starting initialization of EclCovMatrixNtuple Module.");

  // Initializing the output root file
  string dataFileName = m_dataOutFileName + ".root";

  m_rootFile = new TFile(dataFileName.c_str(), "RECREATE");
  m_tree     = new TTree("m_tree", "EclCovMatrixNtuple tree");
  m_nhits = 8736;

  m_tree->Branch("nhits",      &m_nhits,      "nhits/I");
  m_tree->Branch("cellid",     m_cellId,      "cellid[nhits]/I");
  m_tree->Branch("energy",     m_energy,     "energy[nhits]/D");
  m_tree->Branch("alle",       m_allenergy,     "alle[nhits]/D");
  m_tree->Branch("time",       m_time,     "time[nhits]/D");
  m_tree->Branch("theta",      m_theta,       "theta[nhits]/I");
  m_tree->Branch("phi",        m_phi,         "phi[nhits]/I");
  m_tree->Branch("a1",         m_DspHit1,      "a1[nhits][31]/I");
  m_tree->Branch("a2",         m_DspHit2,      "a2[nhits][31]/I");
  m_tree->Branch("base1",      m_baseline1,      "base1[nhits][16]/I");
  m_tree->Branch("base2",      m_baseline2,      "base2[nhits][16]/I");
  m_tree->Branch("baseavg1",   m_baselineAvg1,      "baseavg1[nhits]/D");
  m_tree->Branch("baseavg2",   m_baselineAvg2,      "baseavg2[nhits]/D");
  m_tree->Branch("mv1",         m_maxVal1,     "mv1[nhits]/I");
  m_tree->Branch("mv2",         m_maxVal2,     "mv2[nhits]/I");
  m_tree->Branch("t1",         m_digiTime1,     "t1[nhits]/D");
  m_tree->Branch("t2",         m_digiTime2,     "t2[nhits]/D");
  m_tree->Branch("e1",         m_digiE1,       "e1[nhits]/D");
  m_tree->Branch("e2",         m_digiE2,       "e2[nhits]/D");
  m_tree->Branch("q1",         m_digiQual1,       "q1[nhits]/I");
  m_tree->Branch("q2",         m_digiQual2,       "q2[nhits]/I");

  m_tree->Branch("trig1",      &m_trig1,      "trig1/D");
  m_tree->Branch("trig2",      &m_trig2,      "trig2/D");
  m_tree->Branch("neclhits",      &m_neclhits,      "neclhits/I");

  B2INFO("[ECLDigiStudyModule Module]: Finished initialising");
}

void ECLDigiStudyModule::terminate()
{
  m_rootFile->cd();
  m_tree->Write();
  m_rootFile->Close();
}

void ECLDigiStudyModule::event()
{

  ECLGeometryPar* eclgeo = ECLGeometryPar::Instance();

  int i;
  m_neclhits = m_eclHitsArray.getEntries();
  fill_n(m_cellId, 8736, 0);
  fill_n(m_energy, 8736, 0);
  fill_n(m_allenergy, 8736, 0);
  fill_n(m_time, 8736, 0);
  fill_n(m_theta, 8736, 0);
  fill_n(m_phi, 8736, 0);

  for (int j = 0; j < 8736; j++) {
    fill_n(m_DspHit1[j], 31, 0);
    fill_n(m_DspHit2[j], 31, 0);
    fill_n(m_baseline1[j], 16, 0);
    fill_n(m_baseline2[j], 16, 0);
  }

  fill_n(m_baselineAvg1, 8736, 0);
  fill_n(m_baselineAvg2, 8736, 0);
  fill_n(m_maxVal1, 8736, 0);
  fill_n(m_maxVal2, 8736, 0);

  fill_n(m_digiTime1, 8736, 0);
  fill_n(m_digiTime2, 8736, 0);
  fill_n(m_digiE1, 8736, 0);
  fill_n(m_digiE2, 8736, 0);
  fill_n(m_digiQual1, 8736, 0);
  fill_n(m_digiQual2, 8736, 0);

  m_trig1 = m_eclTrigArray[0]->getTimeTrig();
  m_trig2 = 0;

  for (const auto& hit : m_eclHitsArray) {

    i = hit.getCellId() - 1;
    m_cellId[i]  = i;
    double e = hit.getEnergyDep();
    if (hit.getBackgroundTag() == ECLHit::bg_none) {
      if (m_energy[i] < e)
        m_time[i] = hit.getTimeAve();
      m_energy[i]  += e;
    }
    m_allenergy[i] += e;
    eclgeo->Mapping(i);
    m_theta[i] =  eclgeo->GetThetaID();
    m_phi[i] =  eclgeo->GetPhiID();

    m_digiTime1[i] = m_digiTime2[i] = -10000;
    m_digiE1[i]    = m_digiE2[i]    = -10.;
  }

  for (const auto& digit1 : m_eclDigiArray1) {
    i              = digit1.getCellId() - 1;
    m_digiTime1[i] = digit1.getTimeFit();
    m_digiE1[i]    = digit1.getAmp() / 20000.;
    m_digiQual1[i] = digit1.getQuality();
  }

  for (const auto& digit2 : m_eclDigiArray2) {
    i              = digit2.getCellId() - 1;
    m_digiTime2[i] = digit2.getTimeFit() / 1000.;
    m_digiE2[i]    = digit2.getAmp() / 20000.;
    m_digiQual2[i] = digit2.getQuality();
  }

  for (const auto& dsp : m_eclDspArray1) {
    i = dsp.getCellId() - 1;
    dsp.getDspA(m_DspHit1[i]);
    for (int j = 0; j < 16; j++) {
      m_baseline1[i][j] = m_DspHit1[i][j] - 3000;
      m_baselineAvg1[i] += m_baseline1[i][j];
    }
    m_baselineAvg1[i] /= 16;
    assert(m_DspHit1[i] == (&m_DspHit1[i][0]));
    m_maxVal1[i] = * (max_element(& m_DspHit1[i][16] , (& m_DspHit1[i][16]) + 15));
  }

  for (const auto& dsp : m_eclDspArray2) {
    i = dsp.getCellId() - 1;
    dsp.getDspA(m_DspHit2[i]);
    for (int j = 0; j < 16; j++) {
      m_baseline2[i][j] = m_DspHit2[i][j] - 3000;
      m_baselineAvg2[i] += m_baseline2[i][j];
    }
    m_baselineAvg2[i] /= 16;
    m_maxVal2[i] = * (max_element(& m_DspHit2[i][16] , (& m_DspHit2[i][16]) + 15));
  }

  m_tree->Fill();
}
