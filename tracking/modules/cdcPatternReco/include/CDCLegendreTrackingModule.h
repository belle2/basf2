/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCLEGENDRETRACKINGMODULE_H
#define CDCLEGENDRETRACKINGMODULE_H

#include <framework/core/Module.h>
#include <fstream>

#include <boost/tuple/tuple.hpp>
#include <vector>

namespace Belle2 {
  /**
   *  \addtogroup modules
   *  @{
   *  \addtogroup tracking_modules
   *  \ingroup modules
   *  @{ CDCLegendreTrackingModule @} @}
   */

  /** Forward declaration to avoid including the corresponding header file*/
  class CDCLegendreTrackHit;
  class CDCLegendreTrackCandidate;
  class CDCLegendreTrackFitter;
  class CDCLegendrePatternChecker;
  class CDCLegendreFastHough;
  class CDCLegendreTrackMerger;

  /** CDC tracking module, using Legendre transformation of the drift time circles.
   * This is a module, performing tracking in the CDC. It is based on the paper "Implementation of the Legendre Transform for track segment reconstruction in drift tube chambers" by T. Alexopoulus, et al. NIM A592 456-462 (2008)
   */
  class CDCLegendreTrackingModule: public Module {

  public:

    /** Constructor.
     *  Create and allocate memory for variables here. Add the module parameters in this method.
     */
    CDCLegendreTrackingModule();

    /** Destructor.
     * Use the destructor to release the memory you allocated in the constructor.
     */
    virtual ~CDCLegendreTrackingModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.
     * Use this method to initialize variables, open files etc.
     */
    virtual void initialize();

    /** Called when entering a new run;
     * Called at the beginning of each run, the method gives you the chance to change run dependent constants like alignment parameters, etc.
     */
    virtual void beginRun();

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.
     */
    virtual void event();

    /** This method is called if the current run ends.
     * Use this method to store information, which should be aggregated over one run.
     */
    virtual void endRun();

    /** This method is called at the end of the event processing.
     *  Use this method for cleaning up, closing files, etc.
     */
    virtual void terminate();


  protected:

  private:

    std::string m_cdcHitsColName; /**< Input digitized hits collection name (output of CDCDigitizer module) */
    std::string m_gfTrackCandsColName; /**< Output genfit track candidates collection name*/

    std::vector<CDCLegendreTrackHit*> m_AxialHitList; /**< List of the axial hits used for track finding. This is the vector, which is used for memory management! */
    std::vector<CDCLegendreTrackHit*> m_StereoHitList; /**< List of the stereo hits used for track finding. This is the vector, which is used for memory management! */
    std::list<CDCLegendreTrackCandidate*> m_trackList; /**< List of track candidates. Mainly used for memory management! */
    std::list<CDCLegendreTrackCandidate*> m_fullTrackList; /**< List of track candidates, which consists of 1-9 SLayers */
    std::list<CDCLegendreTrackCandidate*> m_shortTrackList; /**< List of track candidates, which consists of 1-* SLayers, pass through one or more SLayers*/
    std::list<CDCLegendreTrackCandidate*> m_trackletTrackList; /**< List of track candidates, which starts from 3rd or 5th SLayer and pass through one or more SLayers*/
    CDCLegendreTrackFitter* m_cdcLegendreTrackFitter;
    CDCLegendrePatternChecker* m_cdcLegendrePatternChecker;
    CDCLegendreFastHough* m_cdcLegendreFastHough;
    CDCLegendreTrackMerger* m_cdcLegendreTrackMerger;

    int m_threshold; /**< Threshold for votes in the legendre plane, parameter of the module*/
    double m_thresholdUnique; /**< Threshold of unique TrackHits for track building*/
    double m_stepScale; /**< Scale of steps for SteppedHough*/
    int m_initialAxialHits; /**< Initial number of axial hits in the stepped hough algorithm*/
    int m_nbinsTheta; /**< Number of bins in theta, derived from m_maxLevel*/

    double m_resolutionStereo; /**< Total resolution, used for the assignment of stereo hits to tracks*/

    double m_rMin; /**< Minimum in r direction, initialized in initializer list of the module*/
    double m_rMax; /**< Maximum in r direction, initialized in initializer list of the module*/

    int m_maxLevel; /**< Maximum Level of FastHough Algorithm*/

    static constexpr double m_PI = 3.1415926535897932384626433832795; /**< pi is exactly three*/
    static constexpr double m_rc = 0.0176991150442477874; /**< threshold of r, which defines curlers*/

    bool m_reconstructCurler; /**< Stores, curlers shall be reconstructed*/
    bool m_fitTracks; /**< Apply fitting for candidates or not*/
    bool m_earlyMerge; /**< Apply fitting for candidates or not*/

    /** Sort hits for fitting.
     * This method sorts hit indices to bring them in a correct order, which is needed for the fitting
     * @param hitIndices vector with the hit indices, this vector is charged within the function.
     * @param CDCLegendreTrackHits name of the CDCTrackHits array. In this way the sort funtion can get all necessary information about the hits.
     * @param charge estimated charge of the track, which is needed for hits from the same layer to be ordered correctly.
     */
    void sortHits(std::vector<CDCLegendreTrackHit*>& hitIndices, int charge);

    /**
     * Function used in the event function, which contains the search for tracks, calling multiply the Fast Hough algorithm, always just searching for one track and afterwards removin the according hits from the hit list.
     */
    void DoSteppedTrackFinding();

    /**
     * In this function, the stereo hits are assigned to the track candidates.
     */
    void AsignStereoHits();

    /**
     * @brief Function to create a track candidate
     * @param track construction of std::pairs, describing the track candidate by the axial hits, belonging to it and the parameter r and theta
     * @param trackHitList list of all track hits, which are used for track finding. Hits belonging to the track candidate will be deleted from it.
     */
    void createLegendreTrackCandidate(const std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >& track, std::set<CDCLegendreTrackHit*>* trackHitList, std::pair<double, double>& ref_point);


    /**
     * @brief Perform the necessary operations after the track candidate has been constructed
     * @param track The constructed track candidate
     * @param trackHitList list of al track hits, which are used for track finding. Hits belonging to the track candidate will be deleted from it.
     * This function leaves room for other operations like further quality checks or even the actual fitting of the track candidate.
     */
    void processTrack(CDCLegendreTrackCandidate* track, std::set<CDCLegendreTrackHit*>* trackHitList);

    /** Creates GeantFit Track Candidates from CDCLegendreTrackCandidates */
    void createGFTrackCandidates();

    /**
     * @brief Function to split track candidates into 3 groups: long tracks, curlers, and tracklets
     * Long tracks - passed through all superlayers; curlers - starts at 1st superlayer, but not reach 9th; tracklets - some kind of cluster somewhere in CDC, should be merged with other tracklets or tracks
     */
    void checkHitPattern();


    /**
     * @brief Implementation of check for quality criteria after the track candidate was produced.
     */
    bool fullfillsQualityCriteria(CDCLegendreTrackCandidate* cand);

    /**
     * All objects in m_hitList and m_trackList are deleted and the two lists are cleared.
     * Necessary since we cannot use smart pointers up to now.
     */
    void clear_pointer_vectors();

  };

  /**
   * Small helper class to perform sorting of hits in the CDC pattern reco
   */
  struct CDCTracking_SortHit {

    CDCTracking_SortHit(int charge) : m_charge(charge) {}
    bool operator()(CDCLegendreTrackHit* hit1, CDCLegendreTrackHit* hit2);
    int m_charge;
  };

} // end namespace Belle2
#endif


