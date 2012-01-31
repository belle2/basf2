//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TCHit.cc
// Section  : TRG ECL
// Owner    : Yuuji Unno
// Email    : yunno@post.kek.jp
//---------------------------------------------------------------
// Description : A class to represent ECL
//---------------------------------------------------------------
// $Log$
//---------------------------------------------------------------
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include "framework/core/ModuleManager.h"

#include <ecl/geoecl/ECLGeometryPar.h>
#include <ecl/hitecl/ECLSimHit.h>

#include "trg/ecl/TCHit.h"
#include "trg/ecl/TRGECLHit.h"

#include <stdlib.h>
#include <iostream>

using namespace std;
using namespace Belle2;
//
//
//
TCHit::TCHit(){

  _TCMap = new TRGECLTCMapping();

}
//
//
//
TCHit::~TCHit(){

  delete _TCMap;

}
//
//
//
void
TCHit::setup(void)
{
  double E_cell[8736][16]  = {{0}};
  double X_ave[8736][16]   = {{0}};
  double T_ave[8736][16]   = {{0}};
  double Tof_ave[8736][16] = {{0}};
  // Get instance of ecl geometry parameters
  ECLGeometryPar * eclp = ECLGeometryPar::Instance();
  //
  // Loop over all hits of steps
  //
  StoreArray<ECLSimHit> eclArray("ECLSimHits");
  int nHits = eclArray->GetEntriesFast();
  for (int iHits = 0; iHits < nHits; iHits++) {
    // Get a hit
    ECLSimHit* aECLSimHit = eclArray[iHits];
    // Hit geom. info
    int hitCellId      = aECLSimHit->getCellId();
    double hitE        = aECLSimHit->getEnergyDep() * Unit::GeV;
    double hitTOF      = aECLSimHit->getFlightTime() * Unit::ns;

    TVector3 HitInPos  = aECLSimHit->getPosIn();
    TVector3 HitOutPos = aECLSimHit->getPosOut();

    TVector3 PosCell = eclp->GetCrystalPos(hitCellId);
    TVector3 VecCell = eclp->GetCrystalVec(hitCellId);
    double local_pos = (15. - (0.5 * (HitInPos + HitOutPos) - PosCell) * VecCell);
    //
    for (int iECLCell = 0; iECLCell < 8736; iECLCell++) {
      if (hitCellId == iECLCell && hitTOF < 8000) {
        int TimeIndex = (int) hitTOF / 500;
        E_cell[iECLCell][TimeIndex]  = E_cell[iECLCell][TimeIndex]  + hitE;
        X_ave[iECLCell][TimeIndex]   = X_ave[iECLCell][TimeIndex]   + hitE * local_pos;
        Tof_ave[iECLCell][TimeIndex] = Tof_ave[iECLCell][TimeIndex] + hitE * hitTOF;
      }
    } // End loop crsyal 8736
  }
  //
  //
  //
  for (int iECLCell = 0; iECLCell < 8736; iECLCell++) {
    for (int  TimeIndex = 0; TimeIndex < 16; TimeIndex++) {

      if (E_cell[iECLCell][TimeIndex] < 1.0e-9) {continue;}

      X_ave[iECLCell][TimeIndex]   = X_ave[iECLCell][TimeIndex] / E_cell[iECLCell][TimeIndex];
      T_ave[iECLCell][TimeIndex]   = 6.05 + 0.0749 * X_ave[iECLCell][TimeIndex] - 0.00112 * X_ave[iECLCell][TimeIndex] * X_ave[iECLCell][TimeIndex];
      Tof_ave[iECLCell][TimeIndex] = Tof_ave[iECLCell][TimeIndex] / E_cell[iECLCell][TimeIndex];
      // StoreArray<TRGECLHit> eclHitArray(m_eclHitOutColName);
      // m_hitNum = eclHitArray->GetLast() + 1;
      // new(eclHitArray->AddrAt(m_hitNum)) TRGECLHit();
      // eclHitArray[m_hitNum]->setEventId(m_nEvent);
      // eclHitArray[m_hitNum]->setCellId(iECLCell);
      // eclHitArray[m_hitNum]->setEnergyDep(E_cell[iECLCell][TimeIndex]);
      // eclHitArray[m_hitNum]->setTimeAve(T_ave[iECLCell][TimeIndex] + Tof_ave[iECLCell][TimeIndex]);
    }//16 Time interval 16x 500 ns
  }
  //
  //
  //
  double xtal_energy[8736] = {0};
  for (int iXtalIdm = 0; iXtalIdm < 8736; iXtalIdm++) {
    for (int  iTimeIndex = 0; iTimeIndex < 16; iTimeIndex++) {
      if (E_cell[iXtalIdm][iTimeIndex] < 1.0e-9) {continue;}
      xtal_energy[iXtalIdm] =  xtal_energy[iXtalIdm] + E_cell[iXtalIdm][iTimeIndex];
    }
  }
  //
  // Store
  //
  int m_hitNum = 0;
  for (int iXtalIdm = 0; iXtalIdm < 8736; iXtalIdm++) {
    StoreArray<TRGECLHit> eclHitArray("TRGECLHit");
    m_hitNum = eclHitArray->GetLast() + 1;
    new(eclHitArray->AddrAt(m_hitNum)) TRGECLHit();
    eclHitArray[m_hitNum]->setEventId(0);
    eclHitArray[m_hitNum]->setCellId(iXtalIdm + 1);
    eclHitArray[m_hitNum]->setEnergyDep(xtal_energy[iXtalIdm]);
    eclHitArray[m_hitNum]->setTimeAve(0);
  }
  //--------------
  // TC
  //--------------
  double TC_threshold = 0.1; // 100MeV threshold
  double TC_energy_raw[576] = {0};
  double TC_energy_cut[576] = {0};
  for (int iXtalIdm = 0; iXtalIdm < 8736; iXtalIdm++) {
    TC_energy_raw[_TCMap->getTCIdFromXtalId(iXtalIdm+1) - 1] += xtal_energy[iXtalIdm];
  }
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    if (TC_energy_raw[iTCIdm] > TC_threshold) {
      TC_energy_cut[iTCIdm] = TC_energy_raw[iTCIdm];
    }
  }
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    _tcid.push_back(iTCIdm + 1);
    _tcthetaid.push_back(_TCMap->getTCThetaIdFromTCId(iTCIdm+1));
    _tcphiid.push_back(_TCMap->getTCPhiIdFromTCId(iTCIdm+1));
    _tcenergy.push_back(TC_energy_cut[iTCIdm]);
  }
}
//
//===<END>
//
