/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <vxd/dataobjects/VxdID.h> //<pxd/geometry/SensorInfo.h>
#include <genfit/TrackCand.h>
#include <tracking/dataobjects/PXDIntercept.h>
#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /** SVDIntercept stores the U,V coordinates and uncertainties of the intersection of a track
   *   with a PXD sensor.
   */
  class SVDIntercept : public PXDIntercept {

  public:

    /** Default constructor for I/O.
     */
    //    SVDIntercept(){}

    /** Destructor.
     */
    //    ~SVDIntercept() {}

    //! Needed to make the ROOT object storable
    ClassDef(SVDIntercept, 1);
  };
}
