/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclHitMaker/ECLHitModule.h>

//framework headers
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

//ecl package headers
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/dataobjects/HitECL.h>
#include <ecl/geometry/ECLGeometryPar.h>

#include <generators/dataobjects/MCParticle.h>
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
REG_MODULE(ECLHit)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLHitModule::ECLHitModule() : Module()
{
  // Set description
//  setDescription("ECLHitMakerModule");

  setDescription("ECLHitMakerModule");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  // Add parameters
  // I/O
  addParam("ECLHitInputColName", m_inColName, "Input Array // Output from g4sim module", string("ECLSimHits"));
  addParam("ECLHitOutColName", m_eclHitOutColName, "Output of this module//(EventNo,CellId,EnergyDep,TimeAve )", string("ECLHits"));
}

ECLHitModule::~ECLHitModule()
{
}

void ECLHitModule::initialize()
{
  // Initialize variables
  m_nRun    = 0 ;
  m_nEvent  = 0 ;
  m_hitNum = 0;

  StoreArray<ECLSimHit> eclSimArray(m_inColName);
  StoreArray<MCParticle> mcParticles;
  StoreArray<HitECL>::registerPersistent(m_eclHitOutColName);



//  RelationArray mcPartToECLHitRel(mcParticles, eclHitArray);
}

void ECLHitModule::beginRun()
{
}

void ECLHitModule::event()
{

  StoreArray<ECLSimHit> eclSimArray(m_inColName);

  if (!eclSimArray) {
    B2ERROR("Can not find " << m_inColName << ".");
  }

  //---------------------------------------------------------------------
  // Merge the hits in the same cell and save them into ECL signal map.
  //---------------------------------------------------------------------

  // Get number of hits in this event
  int nHits = eclSimArray->GetEntriesFast();

  double E_cell[8736][16] = {{0}};
  double X_ave[8736][16] = {{0}};
  double T_ave[8736][16] = {{0}};
  double Tof_ave[8736][16] = {{0}};

  // Get instance of ecl geometry parameters
  ECLGeometryPar* eclp = ECLGeometryPar::Instance();
  // Loop over all hits of steps
  for (int iHits = 0; iHits < nHits; iHits++) {
    // Get a hit
    ECLSimHit* aECLSimHit = eclSimArray[iHits];


    // Hit geom. info
    int hitCellId       =   aECLSimHit->getCellId();
    double hitE        = aECLSimHit->getEnergyDep() * Unit::GeV;
    double hitTOF         = aECLSimHit->getFlightTime() * Unit::ns;
    TVector3 HitInPos  =   aECLSimHit->getPosIn();
//    TVector3 HitOutPos  =   aECLSimHit->getPosOut();

    TVector3 PosCell =  eclp->GetCrystalPos(hitCellId);
    TVector3 VecCell =  eclp->GetCrystalVec(hitCellId);
    double local_pos = (15. - (HitInPos  - PosCell) * VecCell);

    for (int iECLCell = 0; iECLCell < 8736; iECLCell++) {

      if (hitCellId == iECLCell && hitTOF < 8000) {
        int TimeIndex = (int) hitTOF / 500;
        E_cell[iECLCell][TimeIndex] = E_cell[iECLCell][TimeIndex] + hitE;
        X_ave[iECLCell][TimeIndex] = X_ave[iECLCell][TimeIndex] + hitE * local_pos;
        Tof_ave[iECLCell][TimeIndex] = Tof_ave[iECLCell][TimeIndex] + hitE * hitTOF;

      }
    } // End loop crsyal 8736

  }//for nHit


  for (int iECLCell = 0; iECLCell < 8736; iECLCell++) {
    for (int  TimeIndex = 0; TimeIndex < 16; TimeIndex++) {

      if (E_cell[iECLCell][TimeIndex] > 1.0e-9) {

        X_ave[iECLCell][TimeIndex] = X_ave[iECLCell][TimeIndex] / E_cell[iECLCell][TimeIndex];
        T_ave[iECLCell][TimeIndex]  =  6.05 + 0.0749 * X_ave[iECLCell][TimeIndex] - 0.00112 * X_ave[iECLCell][TimeIndex] * X_ave[iECLCell][TimeIndex];
        Tof_ave[iECLCell][TimeIndex] =  Tof_ave[iECLCell][TimeIndex] / E_cell[iECLCell][TimeIndex];
        StoreArray<HitECL> eclHitArray(m_eclHitOutColName);

//        cout<<iECLCell<<" "<<E_cell[iECLCell][TimeIndex]<<" "<<Tof_ave[iECLCell][TimeIndex] + T_ave[iECLCell][TimeIndex] <<endl;
        if (!eclHitArray) eclHitArray.create();
        m_hitNum = eclHitArray->GetLast() + 1;
        new(eclHitArray->AddrAt(m_hitNum)) HitECL();
        eclHitArray[m_hitNum]->setCellId(iECLCell);
        eclHitArray[m_hitNum]->setEnergyDep(E_cell[iECLCell][TimeIndex]);
        eclHitArray[m_hitNum]->setTimeAve(T_ave[iECLCell][TimeIndex] + Tof_ave[iECLCell][TimeIndex]);
      }//if Energy > 0
    }//16 Time interval 16x 500 ns
  } //store  each crystal hit

//  eclSimArray->Delete();
  m_nEvent++;

}


void ECLHitModule::endRun()
{
  m_nRun++;
}

void ECLHitModule::terminate()
{
}


