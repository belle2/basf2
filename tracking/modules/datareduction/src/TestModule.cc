/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <jan/modules/Test/TestModule.h>
#include <jan/HoughTransformBasic.h>
#include <jan/HoughTransformSine.h>

#include <framework/datastore/StoreArray.h>
#include <svd/dataobjects/SVDHit.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <TVector3.h>

#include <string>


using namespace std;
using namespace Belle2;

REG_MODULE(Test)


TestModule::TestModule() : Module()
{

}

TestModule::~TestModule()
{
}

void TestModule::initialize()
{

}

void TestModule::beginRun()
{

}

void TestModule::event()
{
  //StoreArray<SVDHit> svdArray("SVDHits");
  //B2INFO("Test: " << svdArray.GetEntries());
  /*  StoreArray<SVDSimHit> svdSimArray("SVDSimHits");
    int nHits = svdSimArray->GetEntries();
    B2INFO("Test: " << svdSimArray.GetEntries());

    for (int iHit = 0; iHit < nHits; ++iHit) {
        // Get a simhit
        SVDSimHit* aSimHit = svdSimArray[iHit];
        B2INFO("Ladder ID: " << aSimHit->getLadderID());
        TVector3 posIn = aSimHit->getPosIn();
        B2INFO("PosIn(x): " << posIn[0]);
        B2INFO("PosIn(y): " << posIn[1]);
        B2INFO("PosIn(z): " << posIn[2]);
    } */
  HoughTransformSine bla;
  //HoughTransformBasic* cur;
  //cur = &bla;
  //cur->doHoughSearch();
  bla.doHoughSearch();

}



void TestModule::endRun()
{

}

void TestModule::terminate()
{

}



