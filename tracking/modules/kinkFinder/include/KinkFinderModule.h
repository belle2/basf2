/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
//Object with performing the actual algorithm:
#include <tracking/kinkFinding/fitter/kinkFitter.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/Track.h>

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>

#include <tracking/dbobjects/KinkFinderParameters.h>

#include <string>
#include <memory>

namespace Belle2 {

  /**
   * Kink finder module.
   *
   * Pairs up tracks,
   * tries to find vertices between them.
   *
   * The resulting pairs of tracks are stored as Belle2::Kink.
   */
  class KinkFinderModule : public Module {

  public:

    /** Setting of module description, parameters. */
    KinkFinderModule();

    /** Acknowledgement of destructor. */
    ~KinkFinderModule() override = default;

    /** Registration of StoreArrays, Relations. */
    void initialize() override;

    /** Creates Belle2::Kink from Belle2::Track as described in the class documentation. */
    void event() override;

    /** Prints status summary. */
    void terminate() override;

  private:

    std::string m_arrayNameTrack;  ///< StoreArray name of the Belle2::Track (Input).
    StoreArray <Track> m_tracks;  ///< StoreArray of Belle2::Track.

    std::unique_ptr<kinkFitter> m_kinkFitter;  ///< Object containing the algorithm of Kink creation.
    std::string m_arrayNameRecoTrack;  ///< StoreArray name of the RecoTrack (Input).
    std::string m_arrayNameCopiedRecoTrack;  ///< StoreArray name of the RecoTrack used for creating copies.
    std::string m_arrayNameTFResult;  ///< StoreArray name of the TrackFitResult (In- and Output).
    std::string m_arrayNameKink;  ///< StoreArray name of the Kink (Output).

    DBObjPtr<KinkFinderParameters> m_kinkFinderParameters; ///< kinkFinder parameters Database ObjPtr

    double m_vertexChi2Cut;  ///< Cut on Chi2 for the Kink vertex.
    double m_vertexDistanceCut;  ///< Cut on distance between tracks at the Kink vertex.
    unsigned char m_kinkFitterMode;  ///< Fitter mode.
    bool m_kinkFitterModeSplitTrack; ///< Fitter mode fourth bit to split track.
    double m_precutRho;  ///< Preselection cut on transverse shift from the outer CDC wall for the track ending points.
    double m_precutZ;  ///< Preselection cut on z shift from the outer CDC wall for the track ending points.
    double m_precutDistance;  ///< Preselection cut on distance between ending points of two tracks.
    double m_precutDistance2D;  ///< Preselection cut on 2D distance between ending points of two tracks (for bad z cases).
    double m_precutDistanceSquared;  ///< m_precutDistance squared for convenience
    double m_precutDistance2DSquared;  ///< m_precutDistance2D squared for convenience
    int m_precutSplitNCDCHit; ///< Preselection cut on maximal number of fitted CDC hits for a track candidate to be split.
    double m_precutSplitPValue; ///< Preselection cut on maximal p-value for a track candidate to be split.

    /**
     * Test if the point in space is inside CDC (approximate custom geometry) with respect to shifts from outer wall,
     * passed as parameters of the module.
     * @param pos point in space
     * @return true if the pos is inside the required volume of the CDC;
     * false if outside
     */
    bool ifInCDC(ROOT::Math::XYZVector& pos);

    /**
     * Check if the track can be a mother candidate based on some simple selections.
     * @param recoTrack track of the candidate
     * @return true if recoTrack pass the criteria;
     * false otherwise
     */
    bool preFilterMotherTracks(RecoTrack const* const recoTrack);

    /**
     * Check if the track can be a daughter candidate based on some simple selections.
     * @param recoTrack track of the candidate
     * @return true if recoTrack pass the criteria;
     * false otherwise
     */
    bool preFilterDaughterTracks(RecoTrack const* const recoTrack);

    /**
    * Check if the track can be a candidate to be split based on some simple selections.
    * @param recoTrack recoTrack of the candidate
    * @param track track of the candidate to get the number of fitted CDC hits
    * @return true if recoTrack pass the criteria;
    * false otherwise
    */
    bool preFilterTracksToSplit(RecoTrack const* const recoTrack, Track const* const track);

    /**
     * Track pair preselection based on distance between two tracks with different options.
     * Filter 1: Distance between first point of the daughter and last point of the mother < m_precutDistance (majority).
     * Filter 2: Distance between last point of the daughter and last point of the mother < m_precutDistance
     * (wrong daughter sign; minority but can be increased in future).
     * Filter 3: Distance between the daughter Helix extrapolation to last point of the mother
     * and last point of the mother < m_precutDistance (lost layers for daughter, second largest contribution).
     * Filter 4: 2D distance between first point of the daughter and last point of the mother < m_precutDistance2D
     * (bad daughter resolution recovered by hit reassignment).
     * Filter 5: 2D distance between last point of the daughter and last point of the mother < m_precutDistance2D
     * (bad daughter resolution and wrong daughter sign, almost no events).
     * Filter 6: Distance between the daughter Helix extrapolation to last point of the mother
     * and last point of the mother < m_precutDistance2D
     * (lost layers for daughter combined with bad daughter resolution).
     * @param motherTrack mother track
     * @param daughterTrack daughter track
     * @return flag of the satisfied preselection criteria, or 0 otherwise.
     */
    short isTrackPairSelected(const Track* motherTrack, const Track* daughterTrack);

    /**
     * Kink fitting and storing
     * @param trackMother mother track
     * @param trackDaughter daughter track
     * @param filterFlag flag of the satisfied preselection criteria
     */
    void fitAndStore(const Track* trackMother, const Track* trackDaughter, short filterFlag);

    // counter for KinkFinder statistics
    int m_allStored = 0;    ///< counter for all saved Kinks
    int m_f1Stored = 0;    ///< counter for filter 1 saved Kinks
    int m_f2Stored = 0;    ///< counter for filter 2 saved Kinks
    int m_f3Stored = 0;    ///< counter for filter 3 saved Kinks
    int m_f4Stored = 0;    ///< counter for filter 4 saved Kinks
    int m_f5Stored = 0;    ///< counter for filter 5 saved Kinks
  };
}
