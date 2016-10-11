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
#include <tracking/dataobjects/PXDIntercept.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/pxdDataReductionClasses/ROIGeometry.h>
#include <tracking/pxdDataReductionClasses/ROIinfo.h>

#include <genfit/TrackCand.h>
#include <genfit/KalmanFitter.h>


namespace Belle2 {

  /** This Class implements the interceptor of the SVD tracks on the PXD layers.
   */
  class PXDInterceptor {

  public:
    /** Constructor.
     */
    PXDInterceptor();

    /** Another Constructor.
     *
     *  @TODO: Can the comment explain, why there are these two constructors?
     */
    PXDInterceptor(const ROIinfo* user_theROIinfo);

    /** Destructor.
     */
    virtual ~PXDInterceptor();

    /** Fill the list of PXD intecepts corresponding to the list of track candidates.
     */
    void fillInterceptList(StoreArray<PXDIntercept>* listToBeFilled,
                           const StoreArray<RecoTrack>& trackList,
                           RelationArray* gfTrackToPXDIntercepts);

    /**
     * Set the nuber of iterations of the Kalman Filter to numIterKalmanFilter
     */
    void setNumIterKalmanFilter(int numIterKalmanFilter)
    {
      m_kalmanFilter.setMinIterations(numIterKalmanFilter);
    };


  private:

    genfit::KalmanFitter m_kalmanFilter; /**< kalman filter object to fit the track */

    ROIGeometry m_theROIGeometry; /**< the geometry of the Region Of Interest */

    ROIinfo m_theROIinfo; /**< the ROI info structure */
  };
}
