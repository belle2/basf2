/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/svdROIFinder/SVDROIGeometry.h>
#include <tracking/dataobjects/SVDIntercept.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/pxdDataReductionClasses/ROIinfo.h>
#include <genfit/KalmanFitter.h>

namespace Belle2 {

  /** This Class implements the interceptor of the SVD tracks on the PXD layers.
   */
  class SVDInterceptor {

  public:

    SVDInterceptor(const ROIinfo* theROIinfo, double toleranceZ, double tolerancePhi);

    /** Destructor.
     */
    virtual ~SVDInterceptor();

    /** Fill the list of PXD intecepts corresponding to the list of track candidates.
     */
    void fillInterceptList(StoreArray<SVDIntercept>* listToBeFilled,
                           const StoreArray<RecoTrack>& trackList,
                           RelationArray* recoTrackToSVDIntercepts);


  private:

    const float m_svdLayerRadius[4] = {3.8, 8.0, 11.5, 14.0}; /**< mean SVD layer radius for both layers */

    SVDROIGeometry m_theROIGeometry; /**< the geometry of the Region Of Interest */

    ROIinfo m_theROIinfo; /**< the ROI info structure */

    /** Append the SVDIntercept infos related to the track theTrack to the listToBeFilled.
     */
    void appendIntercepts(StoreArray<SVDIntercept>* interceptList, std::list<ROIDetPlane> planeList, RecoTrack* recoTrack,
                          int recoTrackIndex, RelationArray* recoTrackToSVDIntercepts);

  };
}
