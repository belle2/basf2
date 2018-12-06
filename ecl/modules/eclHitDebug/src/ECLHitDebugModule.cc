/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
//This module
#include <ecl/modules/eclHitDebug/ECLHitDebugModule.h>

//Root
#include <TVector3.h>

//Framework
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

//ECL
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/dataobjects/ECLDebugHit.h>
#include <ecl/geometry/ECLGeometryPar.h>

using namespace std;
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLHitDebug)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLHitDebugModule::ECLHitDebugModule() : Module()
{
  // Set description
  setDescription("ECLHitDebugModule");
  setPropertyFlags(c_ParallelProcessingCertified);

}

ECLHitDebugModule::~ECLHitDebugModule()
{
}

void ECLHitDebugModule::initialize()
{
  // Initialize variables
  m_nRun    = 0 ;
  m_nEvent  = 0 ;
  m_hitNum = 0;
  // CPU time start
  m_timeCPU = clock() * Unit::us;
  m_eclDebugHits.registerInDataStore();
}

void ECLHitDebugModule::beginRun()
{
}

void ECLHitDebugModule::event()
{
  //---------------------------------------------------------------------
  // Merge the hits in the same cell and save them into ECL signal map.
  //---------------------------------------------------------------------

  int const Nbin = 80;
  int const interval = 8000 / Nbin;
  static float E_cell[8736][Nbin];
  static float Tof_ave[8736][Nbin];
  memset(E_cell, 0, sizeof(float) * 8736 * Nbin);
  memset(Tof_ave, 0, sizeof(float) * 8736 * Nbin);

  // Get instance of ecl geometry parameters
  ECLGeometryPar* eclp = ECLGeometryPar::Instance();
  // Loop over all hits of steps
  for (int iHits = 0; iHits < m_eclSimArray.getEntries(); iHits++) {
    // Get a hit
    ECLSimHit* aECLSimHit = m_eclSimArray[iHits];

    // Hit geom. info
    int hitCellId       =   aECLSimHit->getCellId() - 1;
    double hitE        = aECLSimHit->getEnergyDep() * Unit::GeV;
    double hitTOF         = aECLSimHit->getFlightTime() * Unit::ns;
    G4ThreeVector t  =   aECLSimHit->getPosIn();
    TVector3 HitInPos(t.x(), t.y(), t.z());

    const TVector3& PosCell =  eclp->GetCrystalPos(hitCellId);
    const TVector3& VecCell =  eclp->GetCrystalVec(hitCellId);
    double local_pos = (15. - (HitInPos  - PosCell) * VecCell);

    int iECLCell = hitCellId;
    if (hitCellId == iECLCell && hitTOF < 8000) {
      int TimeIndex = (int) hitTOF / interval;
      E_cell[iECLCell][TimeIndex] = E_cell[iECLCell][TimeIndex] + hitE;
      Tof_ave[iECLCell][TimeIndex]  += (6.05 + 0.0749 * local_pos - 0.00112 * local_pos * local_pos + hitTOF) * hitE ;
    }

  }//for nHit


  for (int iECLCell = 0; iECLCell < 8736; iECLCell++) {
    for (int  TimeIndex = 0; TimeIndex < Nbin; TimeIndex++) {

      if (E_cell[iECLCell][TimeIndex] > 0) {
        m_eclDebugHits.appendNew();
        m_hitNum = m_eclDebugHits.getEntries() - 1;
        m_eclDebugHits[m_hitNum]->setCellId(iECLCell + 1);
        m_eclDebugHits[m_hitNum]->setEnergyDep(E_cell[iECLCell][TimeIndex]);
        m_eclDebugHits[m_hitNum]->setTimeAve(Tof_ave[iECLCell][TimeIndex] / E_cell[iECLCell][TimeIndex]);
      }//if Energy > 0
    }//16 Time interval 16x 500 ns
  } //store  each crystal hit

  m_nEvent++;
}


void ECLHitDebugModule::endRun()
{
  m_nRun++;
}

void ECLHitDebugModule::terminate()
{
}
