/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <genfit/RKTrackRep.h>
#include <genfit/Track.h>
#include <genfit/MeasurementFactory.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/StoreArray.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <tracking/pxdDataReductionClasses/PXDInterceptor.h>

using namespace std;
using namespace Belle2;

PXDInterceptor::PXDInterceptor(const ROIinfo* theROIinfo):
  m_theROIinfo(*theROIinfo)
{
}

PXDInterceptor::PXDInterceptor()
{
}

PXDInterceptor::~PXDInterceptor()
{
}

void
PXDInterceptor::fillInterceptList(StoreArray<PXDIntercept>* listToBeFilled, const StoreArray<genfit::TrackCand>& trackCandList,
                                  RelationArray* gfTrackCandToPXDIntercepts)
{

  StoreArray<genfit::TrackCand> trackCandBadStats(m_theROIinfo.badTracksListName);
  StoreArray<genfit::Track> GFtracks(m_theROIinfo.gfTracksListName);
  StoreArray<SVDCluster> svdClusters;

  for (int i = 0; i < trackCandList.getEntries(); ++i) { //loop over all track candidates

    B2DEBUG(1, " %%%%%  Fit track candidate Nr. : " << i + 1);
    genfit::TrackCand* aTrackCandPointer = trackCandList[i];

    genfit::RKTrackRep* trackRep = new genfit::RKTrackRep(aTrackCandPointer->getPdgCode()); //initialize track representation

    genfit::MeasurementFactory<genfit::AbsMeasurement>  factory;
    genfit::MeasurementProducer <SVDCluster, SVDRecoHit>* svdClusterProducer = NULL;
    svdClusterProducer =  new genfit::MeasurementProducer <SVDCluster, SVDRecoHit> (svdClusters.getPtr());
    factory.addProducer(Const::SVD, svdClusterProducer);

    genfit::Track gfTrack(*aTrackCandPointer, factory, trackRep);  //create the track with the corresponding track representation

    try {
      m_kalmanFilter.processTrack(&gfTrack);
    } catch (...) { B2WARNING("track fit failed"); continue; }

    if (! gfTrack.getFitStatus()->isFitConverged()) { B2WARNING("bad track status"); continue; }

    m_theROIGeometry.appendIntercepts(listToBeFilled, &gfTrack, i, gfTrackCandToPXDIntercepts);

    GFtracks.appendNew(gfTrack); //giulia

    //apparently the desctructor of genfit::Track also take care of deleting the RKTrackRep

  } //loop on the track candidate list

} //fillInterceptList
