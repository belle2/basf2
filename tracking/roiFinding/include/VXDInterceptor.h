/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <tracking/roiFinding/ROIGeometry.h>

#include <vector>

namespace Belle2 {
  class RecoTrack;

  /** This Class implements the interceptor of the SVD tracks on the PXD layers.
   */
  template<class aIntercept>
  class VXDInterceptor {

  public:

    /** Constructor */
    VXDInterceptor(double toleranceZ, double tolerancePhi, std::vector<float> layerRadii, VXD::SensorInfoBase::SensorType det)
      : m_layerRadii(layerRadii), m_detector(det)
    {
      B2ASSERT("Can't create VXDIntercepts if no layer radii are provided.", not layerRadii.empty());
      m_theROIGeometry.fillPlaneList(toleranceZ, tolerancePhi, det);
    }

    /** Destructor.
     */
    virtual ~VXDInterceptor() = default;

    /** Fill the list of PXD intecepts corresponding to the list of track candidates.
     */
    void fillInterceptList(StoreArray<aIntercept>* listToBeFilled,
                           const StoreArray<RecoTrack>& trackList,
                           RelationArray* recoTrackToIntercepts);


  private:

    /// Vector containing the radii of the layer, filled in the constructor
    std::vector<float> m_layerRadii = {0};

    /// The detector we are creating intercepts for
    VXD::SensorInfoBase::SensorType m_detector;

    ROIGeometry m_theROIGeometry; /**< the geometry of the Region Of Interest */

    /** Append the Intercept infos related to the track theTrack to the listToBeFilled.
     */
    void appendIntercepts(StoreArray<aIntercept>* interceptList, std::list<ROIDetPlane> planeList, RecoTrack* recoTrack,
                          int recoTrackIndex, RelationArray* recoTrackToIntercepts);

  };
}
