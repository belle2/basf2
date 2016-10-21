/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclHitDebug/ECLHitDebugModule.h>

//framework headers
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

//ecl package headers
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/dataobjects/ECLDebugHit.h>
#include <ecl/geometry/ECLGeometryPar.h>

#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/RelationArray.h>
//root
#include <TVector3.h>

//C++ STL
#include <cstdlib>
#include <iomanip>
#include <math.h>
#include <time.h>
#include <iomanip>
#include <utility> //contains pair

#define PI 3.14159265358979323846


using namespace std;
using namespace boost;
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
//  setDescription("ECLDebugHitMakerModule");

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
  StoreArray<ECLDebugHit>::registerPersistent();

//  RelationArray mcPartToECLHitRel(mcParticles, eclHitArray);
}

void ECLHitDebugModule::beginRun()
{
}

void ECLHitDebugModule::event()
{

  StoreArray<ECLSimHit> eclSimArray;

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
  for (int iHits = 0; iHits < eclSimArray.getEntries(); iHits++) {
    // Get a hit
    ECLSimHit* aECLSimHit = eclSimArray[iHits];


    // Hit geom. info
    int hitCellId       =   aECLSimHit->getCellId() - 1;
    double hitE        = aECLSimHit->getEnergyDep() * Unit::GeV;
    double hitTOF         = aECLSimHit->getFlightTime() * Unit::ns;
    TVector3 HitInPos  =   aECLSimHit->getPosIn();
//    TVector3 HitOutPos  =   aECLSimHit->getPosOut();

    TVector3 PosCell =  eclp->GetCrystalPos(hitCellId);
    TVector3 VecCell =  eclp->GetCrystalVec(hitCellId);
    double local_pos = (15. - (HitInPos  - PosCell) * VecCell);
    //cout<<"DBSimHit"<<m_nEvent<<" " <<cellId<<" "<<hitE<<" "<<hitTOF<<" +  "<<local_pos<<" "<< <<endl;


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

        StoreArray<ECLDebugHit> eclHitArray;
//        cout<<iECLCell<<" "<<E_cell[iECLCell][TimeIndex]<<" "<<Tof_ave[iECLCell][TimeIndex] + T_ave[iECLCell][TimeIndex] <<endl;
        //m_hitNum = eclHitArray->GetLast() + 1;
        //new(eclHitArray->AddrAt(m_hitNum)) ECLDebugHit();
        eclHitArray.appendNew();
        m_hitNum = eclHitArray.getEntries() - 1;
        eclHitArray[m_hitNum]->setCellId(iECLCell + 1);
        eclHitArray[m_hitNum]->setEnergyDep(E_cell[iECLCell][TimeIndex]);
        eclHitArray[m_hitNum]->setTimeAve(Tof_ave[iECLCell][TimeIndex] / E_cell[iECLCell][TimeIndex]);
      }//if Energy > 0
    }//16 Time interval 16x 500 ns
  } //store  each crystal hit

//  eclSimArray->Delete();
  m_nEvent++;
}


void ECLHitDebugModule::endRun()
{
  m_nRun++;
}

void ECLHitDebugModule::terminate()
{
}


