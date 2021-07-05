/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>

#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {
  /** VXDCDCTrackMergerModule a module to merge VXD and CDC tracks
   *
   *  It merges VXD and CDC tracks looking at their intersections
   *  with a cylinder placed at the boundary of the CDC with the VXD.
   */
  class VXDCDCTrackMergerModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    VXDCDCTrackMergerModule();

    /** Use this to initialize resources or memory your module needs.
     *
     *  Also register any outputs of your module (StoreArrays, RelationArrays,
     *  StoreObjPtrs) here, see the respective class documentation for details.
     */
    void initialize() override;

    /** Called once for each event.
     *
     * This is most likely where your module will actually do anything.
     */
    void event() override;

  private:
    /** Radius of the inner CDC wall in centimeters */
    double m_CDC_wall_radius = 16.25;
    /** Maximum distance between extrapolated tracks on the CDC wall */
    double m_merge_radius;

    /** StoreArray name of the VXD Track collection */
    std::string m_VXDRecoTrackColName;
    /** StoreArray name of the CDC Track collection */
    std::string m_CDCRecoTrackColName;

    /** StoreArray of the VXD Track collection */
    StoreArray<RecoTrack> m_VXDRecoTracks;
    /** StoreArray of the CDC Track collection */
    StoreArray<RecoTrack> m_CDCRecoTracks;
  };
}
