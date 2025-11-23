/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
//Object with performing the actual algorithm:
#include <tracking/kinkFinding/fitter/KinkFitter.h>

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
   * Preselects and pairs up tracks and tries to find a vertex between them.
   * Preselects and tries to split tracks that might be combined from two kink tracks.
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

    /** Read parameters from the database and apply to KinkFitter */
    void beginRun() override;

    /** Creates Belle2::Kink from Belle2::Track as described in the class documentation. */
    void event() override;

    /** Prints status summary. */
    void terminate() override;

  private:

    /**
     * Kink fitting and storing
     * @param trackMother mother track
     * @param trackDaughter daughter track
     * @param filterFlag flag of the satisfied preselection criteria
     */
    void fitAndStore(const Track* trackMother, const Track* trackDaughter, const short filterFlag);

    /**
     * Fitter mode 4th bit responsible for turning On/Off track splitting
     * @return true if the track splitting is On and false if it is Off
     */
    bool kinkFitterModeSplitTrack();

    /**
     * Test if the point in space is inside CDC (approximate custom geometry) with respect to shifts from outer wall,
     * passed as parameters of the module.
     * @param pos point in space
     * @return true if the pos is inside the required volume of the CDC;
     * false if outside
     */
    bool ifInCDC(const ROOT::Math::XYZVector& pos);

    /**
     * Check if the track can be a mother candidate based on some simple selections.
     * @param track of the candidate
     * @return true if track pass the criteria;
     * false otherwise
     */
    bool preFilterMotherTracks(Track const* const track);

    /**
     * Check if the track can be a daughter candidate based on some simple selections.
     * @param track of the candidate
     * @return true if track pass the criteria;
     * false otherwise
     */
    bool preFilterDaughterTracks(Track const* const track);

    /**
    * Check if the track can be a candidate to be split based on some simple selections.
    * @param track of the candidate
    * @return true if track pass the criteria;
    * false otherwise
    */
    bool preFilterTracksToSplit(Track const* const track);

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
     * @return flag of the satisfied preselection criteria (1-6 as it was listed above), or 0 otherwise.
     */
    short isTrackPairSelected(const Track* motherTrack, const Track* daughterTrack);

    /**
     * Structure to store the information about forward and backward walls of CDC.
     * CDC has a shape of two trapezoids, so the geometry of forward and backward walls is two sloping lines.
     * This structure handles one sloping line.
     */
    struct CDCForwardBackwardWallLine {

      /**
       * Set parameters of slopping line
       * @param tangent tangent of the sloping line
       * @param offset offset of the sloping line [cm]
       */
      void setLine(const double tangent, const double offset)
      {
        m_tangent = tangent;
        m_offset = offset;
      }

      /**
       * Return the y value of the sloping line based on the x value.
       * It is used in `ifInCDC` function to check if the track point is inside CDC.
       * @param x coordinate [cm]
       * @return y value of the sloping line
       */
      double getLine(const double x) const
      {
        return m_tangent * x + m_offset;
      }

    private:
      double m_tangent; ///< tangent of the sloping line
      double m_offset; ///< offset of the sloping line [cm]
    };

    std::string m_arrayNameTrack;  ///< StoreArray name of the Belle2::Track (Input).
    StoreArray<Track> m_tracks;  ///< StoreArray of Belle2::Track.

    std::unique_ptr<KinkFitter> m_kinkFitter;  ///< Object containing the algorithm of Kink creation.
    std::string m_arrayNameRecoTrack;  ///< StoreArray name of the RecoTrack (Input).
    std::string m_arrayNameCopiedRecoTrack;  ///< StoreArray name of the RecoTrack used for creating copies.
    std::string m_arrayNameTFResult;  ///< StoreArray name of the TrackFitResult (In- and Output).
    std::string m_arrayNameKink;  ///< StoreArray name of the Kink (Output).

    DBObjPtr<KinkFinderParameters> m_kinkFinderParameters; ///< kinkFinder parameters Database ObjPtr

    // CDC and SVD geometry variables
    CDCForwardBackwardWallLine m_cdcForwardBottomWall; ///< Bottom part of forward CDC wall.
    CDCForwardBackwardWallLine m_cdcForwardTopWall; ///< Top part of forward CDC wall.
    CDCForwardBackwardWallLine m_cdcBackwardBottomWall; ///< Bottom part of backward CDC wall.
    CDCForwardBackwardWallLine m_cdcBackwardTopWall; ///< Top part of backward CDC wall.
    static constexpr double m_cdcInnerWithFirstLayerWall = 15; ///< Smaller radius of inner CDC wall [cm].
    ///< It is taken a bit smaller not to loose some events with the first layer of CDC.
    static constexpr double m_cdcInnerWallWithoutFirstLayer = 17; ///< Bigger radius of inner CDC wall [cm].
    ///< It is taken a bit bigger to exclude the first layer of CDC (required in some preselection).
    static constexpr double m_cdcInnerWallWithoutFirstTwoLayers = 18; ///< Second bigger radius of inner CDC wall [cm].
    ///< It is taken a bit bigger to exclude first two layers of CDC (required in some preselection).
    static constexpr double m_cdcOuterWall = 112; ///< Radius of outer CDC wall [cm].
    static constexpr double m_svdBeforeOuterLayer = 12; ///< Radius between two outer SVD layers (10.4 and 13.5 cm) [cm].

    // counter for KinkFinder statistics
    unsigned int m_allStored = 0;    ///< counter for all saved Kinks
    unsigned int m_f1Stored = 0;    ///< counter for filter 1 saved Kinks
    unsigned int m_f2Stored = 0;    ///< counter for filter 2 saved Kinks
    unsigned int m_f3Stored = 0;    ///< counter for filter 3 saved Kinks
    unsigned int m_f4Stored = 0;    ///< counter for filter 4 saved Kinks
    unsigned int m_f5Stored = 0;    ///< counter for filter 5 saved Kinks
  };
}
