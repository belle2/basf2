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
  /** MCTrackMergerModule a module to merge VXD and CDC tracks
   *
   *  This module uses the simulated truth information (MCParticles and their relations) to determine which VXD and CDC tracks
   *  belong to the same particle and should be merged.
   */
  class MCTrackMergerModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    MCTrackMergerModule();

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

    /** This method is called if the current run ends.
     */
    void endRun() override;

  private:
    /** Radius of the inner CDC wall in centimeters */
    double m_CDC_wall_radius = 16.25;
    /** Maximum distance between extrapolated tracks on the CDC wall */
    double m_merge_radius;
    bool m_mcParticlesPresent =
      false; /**< This flag is set to false if there are no MC Particles in the data store (probably data run?) and we can not create MC Reco tracks. */

    int m_totalVXDTracks = 0;    /**< will hold number of input VXD tracks*/
    int m_totalCDCTracks = 0;    /**< will hold number of input CDC tracks*/
    int m_fakeVXDTracks = 0;     /**< will hold number of fake VXD tracks, i.e. not related to MCParticle*/
    int m_fakeCDCTracks = 0;     /**< will hold number of fake CDC traks, i.e. not related to MCParticle*/
    int m_matchedVTXtoCDC = 0;   /**< will hold number of matches from VTX to CDC*/
    int m_matchedVTXtoVTX = 0;   /**< will hold number of matches from VTX to VTX*/
    int m_matchedCDCtoVTX = 0;   /**< will hold number of matches from CDC to VTX*/
    int m_matchedCDCtoCDC = 0;   /**< will hold number of matches from CDC to CDC*/
    int m_matchedTotal = 0;      /**< will hold number of matches*/
    int m_removedCurlers = 0;    /**< will hold number of tracks with hits >1 loop*/


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
