/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/karlsruhe/CDCDummyModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>

#include <framework/logging/Logger.h>

#include <framework/dataobjects/Relation.h>
#include <tracking/dataobjects/Track.h>

using namespace std;
using namespace Belle2;

REG_MODULE(CDCDummy);


CDCDummyModule::CDCDummyModule() : Module()
{
  setDescription("Add all CDCHits up as a single track");

  addParam("CDCRecoHitCollectionName", m_cdcRecoHitCollectionName,
           "Name of the CDCRecoHitCollection", string("CDCRecoHitCollection"));
  addParam("TrackToCDCRecoHitCollectionName", m_trackToCDCRecoHitCollectionName,
           "Name of relation between Track and CDCRecoHits.", string("TrackToCDCRecoHitCollection"));

}

CDCDummyModule::~CDCDummyModule()
{
}

void CDCDummyModule::initialize()
{
}

void CDCDummyModule::beginRun()
{
}

void CDCDummyModule::event()
{

}

void CDCDummyModule::endRun()
{

}

void CDCDummyModule::terminate()
{
}



