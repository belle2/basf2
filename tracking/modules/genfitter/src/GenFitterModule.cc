/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/genfitter/GenFitterModule.h>
#include <tracking/dataobjects/Track.h>

#include <cdc/dataobjects/CDCRecoHit.h>
#include"GFTrack.h"
#include"GFKalman.h"

#include"GFAbsTrackRep.h"
#include<RKTrackRep.h>

#include"GFConstField.h"
#include"GFFieldManager.h"

#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/Relation.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(GenFitter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GenFitterModule::GenFitterModule() : Module()
{
  //Set module properties
  setDescription("Creates CDCRecoHits from CDCHits.");

  //Parameter definition
  addParam("TrackToCDCRecoHitCollectionName", m_trackToCDCRecoHitCollectionName,
           "Name of collection holding the relations between track and CDCRecoHit", string("TrackToCDCRecoHits"));

}


GenFitterModule::~GenFitterModule()
{

}


void GenFitterModule::initialize()
{
}


void GenFitterModule::beginRun()
{
}


void GenFitterModule::event()
{
  StoreArray<Relation> trackToCDCRecoHits(m_trackToCDCRecoHitCollectionName);
  if (trackToCDCRecoHits) {
    StoreArray<CDCRecoHit> cdcRecoHits(trackToCDCRecoHits[0]->getToAccessorInfo());

    for (int iPart = 0; iPart < trackToCDCRecoHits->GetEntriesFast(); iPart++) {
      //aquire the list, which is associated with iPart:
      list<unsigned short int> cdcRecoHitIndices =  trackToCDCRecoHits[iPart]->getToIndices();
      if (cdcRecoHitIndices.empty()) {
        continue;
      }

      // I need some starting values for the Fitter.
      // Later taken from the Track associated with the Relation.
      TVector3 vertex;
      TVector3 momentumDirection(1., 1., 1.);

      GFAbsTrackRep* trackRep = 0;
      trackRep = new RKTrackRep(vertex, momentumDirection, 13);

//      GFTrack fitTrack(trackRep);//initialized with smeared rep

      /*    for(list<unsigned short int>::iterator iter = cdcRecoHitIndices.begin(); iter != cdcRecoHitIndices.end(); iter++){
            fitTrack.addHit(cdcRecoHits[(*iter)]);
          }*/

      GFKalman k;




    }
  }
}


void GenFitterModule::endRun()
{
}

void GenFitterModule::terminate()
{
}
