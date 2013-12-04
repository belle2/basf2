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

#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/PXDIntercept.h>
#include <tracking/pxdDataReductionClasses/ROIDetPlane.h>

#include <genfit/Track.h>
#include <genfit/TrackCand.h>
class ROIDetPlane;

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
    void appendIntercepts(StoreArray<PXDIntercept>* listToBeFilled, genfit::Track* theTrack,
                          int theGFTrackCandIndex, RelationArray* gfTrackCandToPXDIntercepts);

    std::list<ROIDetPlane> m_planeList; /**< list of planes*/

    void fillPlaneList(); /**< fill the list of planes*/
  };
}

