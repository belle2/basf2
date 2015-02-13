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
#include <beast/csi/dataobjects/CsiHit.h>
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

CsIStudyModule::CsIStudyModule() : HistoModule(),
  h_CrystalEdep(NULL),
  h_CrystalSpectrum(NULL),
  h_CrystalRadDose(NULL),
  h_CrystalRadDoseSH(NULL),
  h_NhitCrystal(NULL),
  h_LightYieldCrystal(NULL)
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
  h_CrystalEdep         = new TH1F("Crystal_Edep",  "Energy distribution in each crystal;CellID", 16, -0.5, 15.5);
  h_CrystalSpectrum     = new TH1F("Crystal_Edist",  "Photon energy distribution in all crystals;", 100, 0, 0.1);
  h_NhitCrystal         = new TH1F("Crystal_HitRate", "Number of hit per crystal;CellID; hit/s", 16, -0.5, 15.5);
  h_CrystalRadDose      = new TH1F("Crystal_RadDose", "Crystal Radiation Dose;CellID;Gy/yr", 16, -0.5, 15.5);
  h_CrystalRadDoseSH    = new TH1F("Crystal_RadDose_SH", "Crystal Radiation Dose from SimHits;CellID;Gy/yr", 16, -0.5, 15.5);
  h_LightYieldCrystal   = new TH1F("Crystal_g_yield", "Light yield each crystal;CellID;gamma/sample", 16, -0.5, 15.5);
}

void CsIStudyModule::initialize()
{
  REG_HISTOGRAM   // required to register histograms to HistoManager

  m_hits.isRequired();
  m_simhits.isRequired();
}

void CsIStudyModule::beginRun()
{
}

void CsIStudyModule::endRun()
{
  //  h_LightYieldCrystal->Divide( h_CrystalEdep );
  //  h_LightYieldCrystal->Scale( 1e-6 );
}


void CsIStudyModule::event()
{
  //Loop over CsiHits
  if (m_hits.getEntries() > 0) {
    int hitNum = m_hits.getEntries(); /**< Number of Crystal hits */

    double E_tmp[16] = {0};       /**< Sum energy deposited in each cell */
    double edepSum = 0;           /**< Sum energy deposited in all cells */

    double Mass = 5;             /**< Mass of the crystal (to be replaced with data from the xml once actual mass is known)**/
    double edeptodose = GeVtoJ / Mass * usInYr / Sampletime; /**< Get dose in Gy/yr from Edep */

    /// Actual looping over CsIHits
    for (int i = 0; i < hitNum; i++) {
      CsiHit* aCsIHit = m_hits[i];
      int m_cellID = aCsIHit->getCellId();       /**< Index of the Cell*/
      double edep = aCsIHit->getEnergyDep();     /**< Energy deposited in the current hit */
      //double hitTime = aCsIHit->getTimeAve();    /**< Time of the hit*/

      edepSum += edep;
      E_tmp[m_cellID] += edep;

      // Fill histograms
      h_CrystalSpectrum->Fill(edep);
      h_CrystalEdep->Fill(m_cellID, edep);
      h_CrystalRadDose->Fill(m_cellID, edep * edeptodose);

      //Number of hits per second
      h_NhitCrystal->Fill(m_cellID, 1.0e9 / Sampletime);
    }
  }

  //Loop over CsiSimHits
  if (m_simhits.getEntries() > 0) {
    int hitNum = m_simhits.getEntries(); /**< Number of Crystal hits */

    double Mass = 5;  /**< Mass of the crystal **/
    double  edeptodose = GeVtoJ / Mass * usInYr / Sampletime;/**< Get dose in Gy/yr from Edep */

    /// Actual looping over CsISimHits
    for (int i = 0; i < hitNum; i++) {
      CsiSimHit* aCsIHit = m_simhits[i];
      int m_cellID = aCsIHit->getCellId();       /**< Index of the Cell*/
      double edep = aCsIHit->getEnergyDep();     /**< Energy deposited in the current hit */

      // Fill histograms
      h_CrystalRadDoseSH->Fill(m_cellID, edep * edeptodose);

      // To get the Number of photons per GeV (divide by total edep before plotting)
      if (22 == aCsIHit->getPDGCode()) {
        h_LightYieldCrystal->Fill(m_cellID);
      }
    }
  }
}


void CsIStudyModule::terminate()
{
}


