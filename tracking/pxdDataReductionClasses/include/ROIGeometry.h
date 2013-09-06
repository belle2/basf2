/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ROIGEOMETRY_H_
#define ROIGEOMETRY_H_

#include <list>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/PXDIntercept.h>
#include <RKTrackRep.h>
#include <GFTrackCand.h>
//#include <tracking/dataobjects/ROIDetPlane.h>
#include <tracking/pxdDataReductionClasses/ROIDetPlane.h>

class ROIDetPlane;
class GFTrack;

namespace Belle2 {

  /**
   * This Class append the PXDIntercept infos of a track to the list of intercepts.
   */
  class ROIGeometry {

  public:

    /**
     * Constructor.
     */
    ROIGeometry();

    /**
     * Destructor.
     */
    ~ROIGeometry();

    /**
     * Append the PXDIntercept infos related to the track theTrack to the listToBeFilled
     */
    void appendIntercepts(StoreArray<PXDIntercept>* listToBeFilled, RKTrackRep* theTrack,
                          int theGFTrackCandIndex, RelationArray* gfTrackCandToPXDIntercepts);

    std::list<ROIDetPlane> m_planeList;

    void fillPlaneList();

  };



}


#endif


