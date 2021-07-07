/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <vtx/modules/vtxSpacePointCreator/SimpleVTXSpacePointCreatorModule.h>
#include <vtx/dataobjects/VTXSimHit.h>

#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <tracking/spacePointCreation/SpacePoint.h>

#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/geometry/B2Vector3.h>


//c++
#include <cmath>
#include <boost/foreach.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <utility> // std::pair

// ROOT
#include <TVector3.h>
#include <TRandom.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TFile.h>

using namespace std;
using namespace Belle2;
//using namespace VTX;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SimpleVTXSpacePointCreator)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SimpleVTXSpacePointCreatorModule::SimpleVTXSpacePointCreatorModule() : Module()
{
  // Set module properties
  setDescription("VTX SpacePoint Creator module");
  // TO BE REWORKED

}

SimpleVTXSpacePointCreatorModule::~SimpleVTXSpacePointCreatorModule()
{
}

void SimpleVTXSpacePointCreatorModule::initialize()
{
  B2INFO("Initializing VTXCluster");

  StoreArray<VTXSimHit> VTXSimHits;
  VTXSimHits.isRequired();

  StoreArray<SpacePoint> VTXSpacePoints;
  VTXSpacePoints.registerInDataStore();
  VTXSpacePoints.registerRelationTo(VTXSimHits);

}

void SimpleVTXSpacePointCreatorModule::beginRun()
{

}

void SimpleVTXSpacePointCreatorModule::event()
{

  StoreArray<VTXSimHit>  VTXSimHits;
  StoreArray<SpacePoint> VTXSpacePoints;


  //skip events with no VTXSimHits, but continue the event counter
  if (VTXSimHits.getEntries() == 0) {
    Event++;
    return;
  }


  for (int i = 0; i < VTXSimHits.getEntries(); i++) {
    const VTXSimHit* pHit = VTXSimHits[i];

    // Random values just for compilation, to be reworked
    B2Vector3<double> posErrors(0.0, 0.0, 0.0);
    std::pair<double, double> normalizedLocals(0. , 0.);
    std::pair<bool, bool> clustersAssigneds(true, true);

    // VTX SpacePoints
    SpacePoint* VTXSP = VTXSpacePoints.appendNew(pHit->getPosIn(), posErrors, normalizedLocals,
                                                 clustersAssigneds, pHit->getSensorID(), VXD::SensorInfoBase::SensorType::VXD, pHit->getGlobalTime(), pHit->getGlobalTime());

    VTXSP->addRelationTo(pHit);
  }

  Event++;
}



void SimpleVTXSpacePointCreatorModule::endRun()
{
}

void SimpleVTXSpacePointCreatorModule::terminate()
{
}


