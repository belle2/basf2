/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/pxdDataReductionClasses/ROIDetPlane.h>

#include <list>

namespace Belle2 {

  /** This class appends the PXDIntercept infos of a track to the list of intercepts.
   */
  class ROIGeometry {

  public:
    /** Constructor.
     */
    ROIGeometry();

    /** Destructor.
     */
    ~ROIGeometry();

    /** Append the PXDIntercept infos related to the track theTrack to the listToBeFilled.
     */
    //    void appendIntercepts(StoreArray<PXDIntercept>* listToBeFilled, RecoTrack* theTrack,
    //                          int recoTrackIndex, RelationArray* recoTrackToPXDIntercepts);

    std::list<ROIDetPlane> m_planeList; /**< list of planes*/

    void fillPlaneList(double toleranceZ, double tolerancePhi); /**< fill the list of planes*/

    /** Append to selectedPlanes those in the internal list for which a sensor is near the RecoTrack position */
    void appendSelectedPlanes(std::list<ROIDetPlane>* selectedPlanes, TVector3 recoTrackPosition, int layer);

  };
}

