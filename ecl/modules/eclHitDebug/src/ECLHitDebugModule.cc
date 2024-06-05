/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/modules/eclHitDebug/ECLHitDebugModule.h>

/* ECL headers. */
#include <ecl/dataobjects/ECLDebugHit.h>
#include <ecl/dataobjects/ECLElementNumbers.h>
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/geometry/ECLGeometryPar.h>

/* Basf2 headers. */
#include <framework/gearbox/Unit.h>

/* ROOT headers. */
#include <Math/Vector3D.h>

using namespace std;
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLHitDebug);

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
  static float E_cell[ECLElementNumbers::c_NCrystals][Nbin];
  static float Tof_ave[ECLElementNumbers::c_NCrystals][Nbin];
  memset(E_cell, 0, sizeof(float) * ECLElementNumbers::c_NCrystals * Nbin);
  memset(Tof_ave, 0, sizeof(float) * ECLElementNumbers::c_NCrystals * Nbin);

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
    ROOT::Math::XYZVector HitInPos(t.x(), t.y(), t.z());

    const ROOT::Math::XYZVector& PosCell = eclp->GetCrystalPos(hitCellId);
    const ROOT::Math::XYZVector& VecCell = eclp->GetCrystalVec(hitCellId);
    double local_pos = (15. - (HitInPos  - PosCell).Dot(VecCell));

    int iECLCell = hitCellId;
    if (hitTOF < 8000) {
      int TimeIndex = (int) hitTOF / interval;
      E_cell[iECLCell][TimeIndex] = E_cell[iECLCell][TimeIndex] + hitE;
      Tof_ave[iECLCell][TimeIndex]  += (6.05 + 0.0749 * local_pos - 0.00112 * local_pos * local_pos + hitTOF) * hitE ;
    }

  }//for nHit


  for (int iECLCell = 0; iECLCell < ECLElementNumbers::c_NCrystals; iECLCell++) {
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
