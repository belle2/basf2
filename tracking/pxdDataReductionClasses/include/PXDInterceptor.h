/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXD_INTERCEPTOR_H_
#define PXD_INTERCEPTOR_H_

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <GFTrackCand.h>
#include <GFKalman.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/PXDIntercept.h>
#include <tracking/pxdDataReductionClasses/ROIGeometry.h>

namespace Belle2 {

  /**
   * This Class implements the interceptor of the SVD tracks on the PXD layers.
   */
  class PXDInterceptor {

  public:

    /**
     * Constructor.
     */
    PXDInterceptor();

    /**
     * Destructor.
     */
    virtual ~PXDInterceptor();

    /**
     * Fill the list of PXD intecepts corresponding to the list of track candidates
     */
    void fillInterceptList(StoreArray<PXDIntercept>* listToBeFilled,
                           const StoreArray<GFTrackCand>& trackCandList,
                           RelationArray* gfTrackCandToPXDIntercepts);

    /**
     * Set the nuber of iterations of the Kalman Filter to numIterKalmanFilter
     */
    void setNumIterKalmanFilter(int numIterKalmanFilter) {
      m_kalmanFilter.setNumIterations(numIterKalmanFilter);
    };


  private:

    GFKalman m_kalmanFilter; /**< kalman filter object to fit the track */

    ROIGeometry m_theROIGeometry; /**< the geometry of the Region Of Interest */

  };

}

#endif

