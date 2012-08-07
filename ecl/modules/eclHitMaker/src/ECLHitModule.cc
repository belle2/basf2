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
//#include <framework/dataobjects/Relation.h>
#include <framework/logging/Logger.h>

//ecl package headers
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/dataobjects/HitECL.h>
#include <ecl/geometry/ECLGeometryPar.h>

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
}

void ECLHitModule::beginRun()
{
}

void ECLHitModule::event()
{

}


void ECLHitModule::endRun()
{
  m_nRun++;
}

void ECLHitModule::terminate()
{
}


