/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: beaulieu                                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <beast/csi/modules/CsiModule.h>
#include <beast/csi/dataobjects/CsiSimHit.h>
#include <beast/csi/modules/CsIStudyModule.h>

// ROOT
#include <TVector3.h>
#include <TRandom.h>
#include <TH1.h>
#include <TH2.h>
#include <TFile.h>

using namespace std;
using namespace Belle2;
using namespace csi;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CsIStudy)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CsIStudyModule::CsIStudyModule() : Module()
{
  // Set module properties
  setDescription("Analyze simulations of CsI readings in BEAST");

  // Parameter definitions
  addParam("paramTemplate", m_paramTemplate, "Template of an input parameter. Noop for now.", 0.0);
  addParam("histoFileName", m_inputFileName, "Output file containing histograms",
           string("CsIPlots.root"));

  initialize();
}

CsIStudyModule::~CsIStudyModule()
{
}

void CsIStudyModule::initialize()
{
  m_hits.isRequired();
  m_outputFile = new TFile(m_inputFileName.c_str(), "RECREATE");
  h_EdistTotal  = new TH1F("TotalCSI_E_dist", "Energy distribution in all crystals;GeV", 100, 0, 4);
  h_EdistCrystal = new TH1F("Crystal_E_dist", "Energy distribution in each crystals;GeV", 100, 0, 4);
  h_CrystalRadDose = new TH1F("Crystal_Rad_Dose", "Crystal Radiation Dose;ThetaID;Gy/yr", 16, -0.5, 15.5);
}

void CsIStudyModule::beginRun()
{
}

void CsIStudyModule::endRun()
{
  h_EdistCrystal   ->Write();
  h_CrystalRadDose ->Write();

  m_outputFile->Close();
}

void CsIStudyModule::event()
{


  if (m_hits.getEntries() > 0) {
    int hitNum = m_hits.getEntries(); /**< Number of Crystal hits */

    double E_tmp[16] = {0};       /**< Sum energy deposited in each cell */
    double edepSum = 0;           /**< Sum energy deposited in all cells */


    for (int i = 0; i < hitNum; i++) { // Loop over ECLSimHits
      CsiSimHit* aCsIHit = m_hits[i];
      int m_cellID = aCsIHit->getCellId();       /**< Index of the Cell*/
      double edep = aCsIHit->getEnergyDep();     /**< Energy deposited in the current hit */
      TVector3 hitPosn = aCsIHit->getPosition(); /**< Position of the hit*/


      edepSum += edep;
      E_tmp[m_cellID] += edep;


      double Mass = 5 /**< Mass of the crystal **/;


      // Fill histograms
      h_EdistCrystal->Fill(edep);
      h_EdistTotal->Fill(edepSum);
      h_CrystalRadDose->Fill(m_cellID, edep * GeVtoJ / Mass * usInYr / Sampletime);

    }
  }
}

void CsIStudyModule::terminate()
{
}


