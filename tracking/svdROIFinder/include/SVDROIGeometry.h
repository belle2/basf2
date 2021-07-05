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

  /** This class appends the SVDIntercept infos of a track to the list of intercepts.
   */
  class SVDROIGeometry {

  public:
    /** Constructor.
     */
    SVDROIGeometry();

    /** Destructor.
     */
    ~SVDROIGeometry();

    std::list<ROIDetPlane> m_planeList; /**< list of planes*/

    void fillPlaneList(double toleranceZ, double tolerancePhi); /**< fill the list of planes*/

    /** appends the interesting planes*/
    void appendSelectedPlanes(std::list<ROIDetPlane>* selectedPlanes, TVector3 recoTrackPosition, int layer);

  };
}

