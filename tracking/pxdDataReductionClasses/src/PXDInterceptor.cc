/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <RKTrackRep.h>
#include <GFTrack.h>
#include <GFRecoHitFactory.h>
#include <GFRecoHitProducer.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/StoreArray.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <tracking/pxdDataReductionClasses/PXDInterceptor.h>

using namespace std;
using namespace Belle2;

PXDInterceptor::PXDInterceptor()
{
}

PXDInterceptor::~PXDInterceptor()
{
}

void
PXDInterceptor::fillInterceptList(StoreArray<PXDIntercept>* listToBeFilled, const StoreArray<GFTrackCand>& trackCandList,
                                  RelationArray* gfTrackCandToPXDIntercepts)
{

  StoreArray<SVDCluster> svdClusters;

  for (int i = 0; i < trackCandList.getEntries(); ++i) { //loop over all track candidates

    B2DEBUG(1, " %%%%%  Fit track candidate Nr. : " << i + 1);
    GFTrackCand* aTrackCandPointer = trackCandList[i];

    RKTrackRep* trackRep = new RKTrackRep(aTrackCandPointer); //initialize track representation and give the seed helix parameters and cov and the pdg code to the track fitter

    GFTrack gfTrack(trackRep);  //create the track with the corresponding track representation
    gfTrack.setCandidate(*aTrackCandPointer);
    GFRecoHitFactory factory;
    GFRecoHitProducer <SVDCluster, SVDRecoHit>* svdClusterProducer = NULL;
    svdClusterProducer =  new GFRecoHitProducer <SVDCluster, SVDRecoHit> (svdClusters.getPtr());
    factory.addProducer(Const::SVD, svdClusterProducer);

    //use the factory to create RecoHits for all Hits stored in the track candidate
    vector <GFAbsRecoHit*> factoryHits = factory.createMany(*aTrackCandPointer);
    //add created hits to the track
    gfTrack.addHitVector(factoryHits);
    gfTrack.setCandidate(*aTrackCandPointer);

    try {
      m_kalmanFilter.processTrack(&gfTrack);
    } catch (...) { B2WARNING("track fit failed"); continue; }

    if (trackRep->getStatusFlag() != 0) { B2WARNING("bad track status"); continue; }

    m_theROIGeometry.appendIntercepts(listToBeFilled, trackRep, i, gfTrackCandToPXDIntercepts);

    //apparently the desctructor of GFTrack also take care of deleting the RKTrackRep

  } //loop on the track candidate list

} //fillInterceptList
