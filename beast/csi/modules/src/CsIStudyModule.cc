/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: beaulieu                                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>

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

CsIStudyModule::CsIStudyModule() : HistoModule()
{
  // Set module properties
  setDescription("Analyze simulations of CsI readings in BEAST. Requires HistoManager module.");

  // Parameter definitions
  addParam("paramTemplate", m_paramTemplate, "Template of an input parameter. Noop for now.", 0.0);

}

CsIStudyModule::~CsIStudyModule()
{
}

void CsIStudyModule::defineHisto()
{
  h_EdistTotal  = new TH1F("TotalCSI_E_dist", "Energy distribution in all crystals;GeV", 100, 0, 4);
  h_EdistCrystal = new TH1F("Crystal_E_dist", "Energy distribution in each crystals;GeV", 100, 0, 0.1);
  h_NhitCrystal  = new TH1F("Crystal_g_yield", "Light yield each  crystal;ThetaID:gamma/GeV", 16, -0.5, 15.5);
  h_CrystalRadDose = new TH1F("Crystal_Rad_Dose", "Crystal Radiation Dose;ThetaID;Gy/yr", 16, -0.5, 15.5);
}

void CsIStudyModule::initialize()
{
  REG_HISTOGRAM   // required to register histograms to HistoManager

  m_hits.isRequired();
}

void CsIStudyModule::beginRun()
{
}

void CsIStudyModule::endRun()
{
  //h_NhitCrystal    ->Divide(h_CrystalRadDose);
}


void CsIStudyModule::event()
{



  if (m_hits.getEntries() > 0) {
    int hitNum = m_hits.getEntries(); /**< Number of Crystal hits */

    double E_tmp[16] = {0};       /**< Sum energy deposited in each cell */
    double edepSum = 0;           /**< Sum energy deposited in all cells */

    double Mass = 5;  /**< Mass of the crystal **/
    double  edeptodose = GeVtoJ / Mass * usInYr / Sampletime;

    for (int i = 0; i < hitNum; i++) { // Loop over ECLSimHits
      CsiSimHit* aCsIHit = m_hits[i];
      int m_cellID = aCsIHit->getCellId();       /**< Index of the Cell*/
      double edep = aCsIHit->getEnergyDep();     /**< Energy deposited in the current hit */
      TVector3 hitPosn = aCsIHit->getPosition(); /**< Position of the hit*/

      edepSum += edep;
      E_tmp[m_cellID] += edep;


      // Fill histograms
      h_EdistCrystal->Fill(edep);
      h_EdistTotal->Fill(edepSum);
      h_CrystalRadDose->Fill(m_cellID, edep * edeptodose);
      h_NhitCrystal->Fill(m_cellID, edeptodose);
    }

  }
}

void CsIStudyModule::terminate()
{
}


