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
#include <mdst/dataobjects/MCParticle.h>
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

    /** Called once for a VXD or CDC only RecoTrack Store array in an event.
     *
     * The methode loops over all tracks in the tracks array. The relation of hits to MCParticles
     * and TrueHits/SimHits are followed to determine the minimum time of fligt over all hits and
     * the Store array index of the the MCParticle with most hits on track. The information is
     * pushed into vectors trackMinToF and trackMCParticles.
     */
    void analyzeTrackArray(StoreArray<RecoTrack>& recoTracks, const StoreArray<MCParticle>& mcParticles,
                           std::vector<int>& trackMCParticles, std::vector<double>& trackMinToF);

    /** Called once for a VXD or CDC only RecoTrack Store array in an event.
     *
     * The methode loops over all tracks in the tracks array. In case there are no hits
     * linked to an MCParticle, or less than 66% of hits are linked to the most frequent particle,
     * the tracks quality indicator will be set to zero and the track will be ignored in the rest of
     * the processing.
     */
    void cleanTrackArray(StoreArray<RecoTrack>& recoTracks, std::vector<int>& trackMCParticles, bool isVXD);

    /** Called once for a VXD or CDC only RecoTrack Store array in an event.
     *
     * The methode removes clones on the tracks array based on the time of flight of segments belonging
     * to the same MCParticle. The vectors tracksMinToF and tracksMCParticles filled by previous call to
     * analyzeTrackArray should be used. If more than one track belonging to the same MCParticle
     * in the tracking volume (CDC or VXD) is found, only the track with smaller min time of flight will be
     * kept. The quality indicator of the other track and all related tracks will be set to zero.
     */
    void removeClonesFromTrackArray(StoreArray<RecoTrack>& recoTracks, std::vector<int>& tracksMCParticles,
                                    std::vector<double>& tracksMinToF, const std::string& relatedTracksColumnName,
                                    bool isVXD);

    /** Called once for to merge VXD and CDC only tracks an event.
     *
     * The methode merges (makes relations) between pairs of a CDC and a VXD track belonging to the same
     * MCParticle. Tracks with QI of zero are ignored. Incorrect existing relations get removed.
     */
    void mergeVXDAndCDCTrackArrays(StoreArray<RecoTrack>& cdcTracks,
                                   const std::vector<int>& cdcTrackMCParticles,
                                   const std::vector<double>& cdcTrackMinToF,
                                   StoreArray<RecoTrack>& vxdTracks,
                                   const std::vector<int>& vxdTrackMCParticles,
                                   const std::vector<double>& vxdTrackMinToF);


    bool m_mcParticlesPresent =
      false; /**< This flag is set to false if there are no MC Particles in the data store (probably data run?) and we can not create MC Reco tracks. */

    int m_totalVXDTracks = 0;    /**< will hold number of input VXD tracks*/
    int m_totalCDCTracks = 0;    /**< will hold number of input CDC tracks*/
    int m_fakeVXDTracks = 0;     /**< will hold number of fake VXD tracks, i.e. not related to MCParticle*/
    int m_fakeCDCTracks = 0;     /**< will hold number of fake CDC traks, i.e. not related to MCParticle*/
    int m_matchedTotal = 0;      /**< will hold number of matches*/
    int m_removedVXDCurlers = 0;    /**< will hold number of VXD tracks from higher loops*/
    int m_removedCDCCurlers = 0;    /**< will hold number of CDC tracks from higher loops*/
    int m_foundRelatedTracks = 0;    /**< will hold number of CDC tracks with existing relation*/
    int m_foundButWrongRelations = 0;  /**< will hold number of CDC tracks with existing relations that where removed (wrong)*/
    int m_foundCorrectlyRelatedTracks = 0;    /**< will hold number of CDC tracks with existing correct relation*/
    int m_foundWronglyRelatedTracks_FAKE = 0;    /**< will hold number of CDC tracks with existing but wrong relation to fake*/
    int m_foundWronglyRelatedTracks_BADORDER =
      0;    /**< will hold number of CDC tracks with existing but wrong sorted relation (VXD track after CDC track)*/
    int m_foundWronglyRelatedTracks_OTHER =
      0;   /**< will hold number of CDC tracks with existing but wrong relation to other signal track*/

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
