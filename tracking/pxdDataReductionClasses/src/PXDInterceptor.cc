/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/RecoTrack.h>
#include <genfit/MeasurementFactory.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/StoreArray.h>
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
PXDInterceptor::fillInterceptList(StoreArray<PXDIntercept>* listToBeFilled, const StoreArray<RecoTrack>& trackList,
                                  RelationArray* recoTrackToPXDIntercepts)
{

  for (int i = 0; i < trackList.getEntries(); ++i) { //loop over all tracks

    B2DEBUG(1, " %%%%%  track candidate Nr. : " << i + 1);

    m_theROIGeometry.appendIntercepts(listToBeFilled, trackList[i], i, recoTrackToPXDIntercepts);

  } //loop on the track list

} //fillInterceptList
