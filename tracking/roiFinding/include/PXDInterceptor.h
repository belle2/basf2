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


namespace Belle2 {
  class PXDIntercept;
  class RecoTrack;

  /** The PXDInterceptor class fills a StoreArray of PXDIntercepts that will be used to define the PXD ROIs.
   * The class assumes that the tracks are coming from the IP and that the genfit propagation direction is
   * inward bound. (i.e. it does not try to find intercepts on both ends of the track.)
   */
  class PXDInterceptor {

  public:

    /** Class Constructor.
     */
    PXDInterceptor(double toleranceZ, double tolerancePhi);

    /** Destructor.
     */
    virtual ~PXDInterceptor();

    /** Fill the list of PXD intecepts corresponding to the list of track candidates.
     */
    void fillInterceptList(StoreArray<PXDIntercept>* listToBeFilled,
                           const StoreArray<RecoTrack>& trackList,
                           RelationArray* recoTrackToPXDIntercepts);

  private:

    const float m_pxdLayerRadius[2] = {1.42854, 2.21218}; /**< mean PXD layer radius for both layers */

    ROIGeometry m_theROIGeometry; /**< the geometry of the Region Of Interest */

    /** Append the PXDIntercept infos related to the track theTrack to the listToBeFilled.
     */
    void appendIntercepts(StoreArray<PXDIntercept>* interceptList, std::list<ROIDetPlane> planeList, RecoTrack* recoTrack,
                          int recoTrackIndex, RelationArray* recoTrackToPXDIntercepts);


  };
}
