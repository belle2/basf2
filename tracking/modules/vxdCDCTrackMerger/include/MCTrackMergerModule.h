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
     * pushed into vectors trackMinToF and trackMCParticleIndices.
     */
    void analyzeTrackArray(const StoreArray<RecoTrack>& recoTracks,
                           std::vector<int>& trackMCParticleIndices, std::vector<double>& trackMinToF);

    /** Called once for a VXD or CDC only RecoTrack Store array in an event.
     *
     * The methode loops over all tracks in the tracks array. In case there are no hits
     * linked to an MCParticle, or less than 66% of hits are linked to the most frequent particle,
     * the tracks quality indicator will be set to zero and the track will be ignored in the rest of
     * the processing.
     */
    void cleanTrackArray(StoreArray<RecoTrack>& recoTracks, const std::vector<int>& trackMCParticleIndices, bool isVXD);

    /** Called once for a VXD or CDC only RecoTrack Store array in an event.
     *
     * The methode removes clones on the tracks array based on the time of flight of segments belonging
     * to the same MCParticle. The vectors tracksMinToF and tracksMCParticleIndices filled by previous call to
     * analyzeTrackArray should be used. If more than one track belonging to the same MCParticle
     * in the tracking volume (CDC or VXD) is found, only the track with smaller min time of flight will be
     * kept. The quality indicator of the other track and all related tracks will be set to zero.
     */
    void removeClonesFromTrackArray(StoreArray<RecoTrack>& recoTracks, const std::vector<int>& tracksMCParticleIndices,
                                    const std::vector<double>& tracksMinToF, const std::string& relatedTracksColumnName,
                                    bool isVXD);

    /** Called once for a VXD or CDC only RecoTrack Store array in an event.
     *
     * The methode inspects the relations from CDC tracks to VXD tracks and vice versa. Incorrect relations
     * get removed. The dicision is based on the best matched MC particle and the time of flight to determine
     * the weight of relations (-> order of track segments in RelatedTracksCombiner).
     */
    void checkRelatedTrackArrays(const std::vector<int>& cdcTrackMCParticleIndices,
                                 const std::vector<double>& cdcTrackMinToF,
                                 const std::vector<int>& vxdTrackMCParticleIndices,
                                 const std::vector<double>& vxdTrackMinToF);

    /** Called once for to merge VXD and CDC only tracks an event.
     *
     * The methode merges (makes relations) between pairs of a CDC and a VXD track belonging to the same
     * MCParticle. Tracks with QI of zero are ignored. Incorrect existing relations get removed.
     */
    void mergeVXDAndCDCTrackArrays(const std::vector<int>& cdcTrackMCParticleIndices,
                                   const std::vector<double>& cdcTrackMinToF,
                                   const std::vector<int>& vxdTrackMCParticleIndices,
                                   const std::vector<double>& vxdTrackMinToF);

    int m_totalVXDTracks = 0;    /**< will hold number of input VXD tracks*/
    int m_totalCDCTracks = 0;    /**< will hold number of input CDC tracks*/
    int m_fakeVXDTracks = 0;     /**< will hold number of fake VXD tracks, i.e. not related to MCParticle*/
    int m_fakeCDCTracks = 0;     /**< will hold number of fake CDC traks, i.e. not related to MCParticle*/
    int m_matchedTotal = 0;      /**< will hold number of matches*/
    int m_removedVXDClones = 0;    /**< will hold number of removed VXD clone tracks*/
    int m_removedCDCClones = 0;    /**< will hold number of removed CDC clone tracks*/
    int m_foundRelatedTracks = 0;    /**< will hold number of CDC tracks with existing relation*/
    int m_foundButWrongRelations = 0;  /**< will hold number of CDC tracks with existing relations that where removed (wrong)*/
    int m_foundCorrectlyRelatedTracks = 0;    /**< will hold number of CDC tracks with existing correct relation*/

    /** StoreArray name of the VXD Track collection */
    std::string m_VXDRecoTrackColName;
    /** StoreArray name of the CDC Track collection */
    std::string m_CDCRecoTrackColName;
    /** StoreArray name of the MCParticle collection */
    std::string m_MCParticleColName;

    /** StoreArray of MCParticle collection */
    StoreArray<MCParticle> m_MCParticles;
    /** StoreArray of the VXD Track collection */
    StoreArray<RecoTrack> m_VXDRecoTracks;
    /** StoreArray of the CDC Track collection */
    StoreArray<RecoTrack> m_CDCRecoTracks;
  };
}
