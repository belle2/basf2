/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/dataobjects/SorterCDCHit.h>
#include <tracking/dataobjects/SorterVXDHit.h>

#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/HitPatternVXD.h>

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>

#include <genfit/Track.h>

#include <set>
#include <vector>
#include <string>

class FitConfiguration;

namespace Belle2 {
  /** This is the Reconstruction Event-Data Model Track.
   *
   *  This class collects hits, performs fits and saves the Track parameters.
   *  Note: This class is still experimental.
   *  Totally missing:
   *   - Fitting functionality
   *   - Storing of which Trackfinders have contributed hits to the RecoTrack
   *     and the exact relation with the HitInfoManager.
   *   - Sorting magic.
   */
  class RecoTrack : public Belle2::RelationsInterface <genfit::Track> {
  public:
    /** Constructor defining the used hit types.
     *
     *  @param cdcHitsName     Name of StoreArray with CDCHits to be used.
     *  @param svdHitsName     Name of StoreArray with either SVDClusters or SVDTrueHits.
     *  @param pxdHitsName     Name of StoreArray with either PXDClusters or PXDTrueHits.
     */
    RecoTrack(const std::string& cdcHitsName = "CDCHits",
              const std::string& svdHitsName = "SVDClusters",
              const std::string& pxdHitsName = "PXDClusters") :
      m_cdcHitsName(cdcHitsName), m_svdHitsName(svdHitsName), m_pxdHitsName(pxdHitsName),
      m_hitPatternCDCInitializer(0), m_hitPatternVXDInitializer(0)
    {}

    /** Defining the Sorter used for CDC hits. */
    typedef SorterCDCHit SorterCDCHits;

    /** Defining the Sorter used for SVD hits. */
    typedef SorterVXDHit SorterVXDHits;

    //-------------------------------------- CDC Hit Handling ---------------------------
    //-----------------------------------------------------------------------------------
    /** Replacing the existing CDC indices with new indices.
     *
     *  @param pseudocharge  Shall those indices be used for the positive (or negative)
     *                       arm of the track.
     *  @sa m_cdcHitIndicesPositive
     */
    void setCDCHitIndices(const std::set< std::pair < unsigned short, short> >& cdcHitIndices,
                          const short pseudocharge) {
      pseudocharge > 0 ?
      (m_cdcHitIndicesPositive = cdcHitIndices) : (m_cdcHitIndicesNegative = cdcHitIndices);
    }

    /** Adding a single index to the set of CDC Indices.
     *
     *  @param   pseudoCharge  Shall this index be used for the positive (or negative)
     *                         arm of the track.
     *  @return  True, if the index was inserted,
     *           false, if the index already existed before.
     */
    bool addCDCHitIndex(const std::pair< unsigned short, short> cdcHitIndex,
                        const short pseudoCharge) {
      return (pseudoCharge > 0 ?
              (m_cdcHitIndicesPositive.insert(cdcHitIndex)) :
              (m_cdcHitIndicesNegative.insert(cdcHitIndex))).second;
    }

    /** Adding several CDC indices at the same time.
     *
     *  In this case a vector is chosen as input, as this might be the favorable type at
     *  creation of the hit indices.
     *  @param  pseudoCharge  Shall those indices be used for the positive (or negative)
     *                        arm of the track.
     */
    void addCDCHitIndices(const std::vector< std::pair < unsigned short, short> >& cdcHitIndices,
                          const short pseudoCharge) {
      pseudoCharge > 0 ?
      m_cdcHitIndicesPositive.insert(cdcHitIndices.begin(), cdcHitIndices.end()) :
      m_cdcHitIndicesNegative.insert(cdcHitIndices.begin(), cdcHitIndices.end());
    }

    /** Has the track any CDC Hits? */
    bool hasCDCHits() {
      return !(m_cdcHitIndicesPositive.empty() and m_cdcHitIndicesNegative.empty());
    }

    /** Fill HitPatternCDC with hits from track arm indicated by pseudoCharge. */
    void fillHitPatternCDC(const short pseudoCharge);

    /** Getter for the hit pattern of the CDC. */
    HitPatternCDC getHitPatternCDC() {
      return HitPatternCDC(m_hitPatternCDCInitializer);
    }

    //-------------------------------------- VXD Hit Handling ---------------------------
    //-----------------------------------------------------------------------------------
    /** Replacing the existing SVD indices with new indices.
     *
     *  @param pseudocharge  Shall those indices be used for the positive (or negative)
     *                       arm of the track.
     */
    void setSVDHitIndices(const std::set< unsigned short >& svdHitIndices,
                          const short pseudocharge) {
      pseudocharge > 0 ?
      (m_svdHitIndicesPositive = svdHitIndices) : (m_svdHitIndicesNegative = svdHitIndices);
    }

    /** Replacing the existing PXD indices with new indices.
     *
     *  @param pseudocharge  Shall those indices be used for the positive (or negative)
     *                       arm of the track.
     */
    void setPXDHitIndices(const std::set< unsigned short >& pxdHitIndices,
                          const short pseudocharge) {
      pseudocharge > 0 ?
      (m_pxdHitIndicesPositive = pxdHitIndices) : (m_pxdHitIndicesNegative = pxdHitIndices);
    }

    /** Adding a single index to the set of SVD Indices.
     *
     *  @param   pseudoCharge  Shall this index be used for the positive (or negative)
     *                         arm of the track.
     *  @return  True, if the index was inserted,
     *           false, if the index already existed before.
     */
    bool addSVDHitIndex(const unsigned short svdHitIndex,
                        const short pseudoCharge) {
      return (pseudoCharge > 0 ?
              (m_svdHitIndicesPositive.insert(svdHitIndex)) :
              (m_svdHitIndicesNegative.insert(svdHitIndex))).second;
    }

    /** Adding a single index to the set of SVD Indices.
     *
     *  @param   pseudoCharge  Shall this index be used for the positive (or negative)
     *                         arm of the track.
     *  @return  True, if the index was inserted,
     *           false, if the index already existed before.
     */
    bool addPXDHitIndex(const unsigned short pxdHitIndex,
                        const short pseudoCharge) {
      return (pseudoCharge > 0 ?
              (m_pxdHitIndicesPositive.insert(pxdHitIndex)) :
              (m_pxdHitIndicesNegative.insert(pxdHitIndex))).second;
    }

    /** Adding several SVD indices at the same time.
     *
     *  In this case a vector is chosen as input, as this might be the favorable type at
     *  creation of the hit indices.
     *  @param  pseudoCharge  Shall those indices be used for the positive (or negative)
     *                        arm of the track.
     */
    void addSVDHitIndices(const std::vector< unsigned short >& svdHitIndices,
                          const short pseudoCharge) {
      pseudoCharge > 0 ?
      m_svdHitIndicesPositive.insert(svdHitIndices.begin(), svdHitIndices.end()) :
      m_svdHitIndicesNegative.insert(svdHitIndices.begin(), svdHitIndices.end());
    }

    /** Adding several PXD indices at the same time.
     *
     *  In this case a vector is chosen as input, as this might be the favorable type at
     *  creation of the hit indices.
     *  @param  pseudoCharge  Shall those indices be used for the positive (or negative)
     *                        arm of the track.
     */
    void addPXDHitIndices(const std::vector< unsigned short >& pxdHitIndices,
                          const short pseudoCharge) {
      pseudoCharge > 0 ?
      m_svdHitIndicesPositive.insert(pxdHitIndices.begin(), pxdHitIndices.end()) :
      m_svdHitIndicesNegative.insert(pxdHitIndices.begin(), pxdHitIndices.end());
    }

    /** Has the track SVD hits? */
    bool hasSVDHits() {
      return !(m_svdHitIndicesPositive.empty() and m_svdHitIndicesNegative.empty());
    }

    /** Has the track PXD hits? */
    bool hasPXDHits() {
      return !(m_pxdHitIndicesPositive.empty() and m_pxdHitIndicesNegative.empty());
    }

    /** Has the track VXD hits? */
    bool hasVXDHits() {
      return (hasSVDHits() or hasPXDHits());
    }

    /** Fill HitPatternVXD with hits from track arm indicated by pseudoCharge. */
    void fillHitPatternVXD(const short pseudoCharge);

    /** Getter for the hit pattern of the VXD. */
    HitPatternVXD getHitPatternVXD() {
      return HitPatternVXD(m_hitPatternVXDInitializer);
    }

    //-------------------------------------- General hit handling -----------------------
    //-----------------------------------------------------------------------------------
    /** Eliminating indices from this RecoTrack.
     *
     *  @param  pseudoCharge  Defines which arm of the track is reset.
     *                        If set to 0, both arms will be reset.
     *  @param detector       Says which detector should be cleared.
     *                        If invalid detector is chose, this means
     *                        all detectors get cleared.
     */
    void resetHitIndices(const short pseudoCharge,
                         const Const::EDetector detector = Const::EDetector::invalidDetector);

    //-------------------------------------- Fitting ------------------------------------
    /** Fit the track with a fitConfiguration object. */
    void fitTrack(const FitConfiguration& fitConfiguration) {
      /* This function has to move to the source file of course, but will consist of
       * - recreating RecoHits
       * - configure the fitter
       * - hand "this" over to the fitter
       *
       *  We want to do it this way, so we can save fit configurations belonging to
       *  certain genfit::TrackReps.
       *  As we want to have potentially information from the fit in this class, I'm afraid
       *  this little overloading of the RecoTrack class can't be avoided entirely.
       */
    }

  private:
    //-------------------------------------- Hit Pattern Handling -----------------------
    /** Member for initializing the information about hits in the CDC.
     *
     *  @sa HitPatternCDC
     */
    unsigned long m_hitPatternCDCInitializer;

    /** Member for initializing the information about hits in the VXD.
     *
     *  @sa HitPatternVXD
     */
    unsigned int m_hitPatternVXDInitializer;

    //-------------------------------------- Hit Indices Storage ------------------------
    //-----------------------------------------------------------------------------------
    /** Name of array of CDCHits to be accessed. */
    std::string m_cdcHitsName;

    /** Defining the order of the CDCHits within one arm for fitting. */
    SorterCDCHits m_sorterCDCHits; //! Don't stream this pointer to file.

    /** CDC indices and right/left ambiguity resolution for the positive arm of the track.
     *
     *  The first element of the pair indicates the index, the second element is used
     *  for the left right/left disambiguation.
     *  FIXME What stands for left, what stands for right?
     */
    std::set< std::pair < unsigned short, short> > m_cdcHitIndicesPositive;

    /** CDC indices and right/left ambiguity resolution for the negative arm of the track.
     *
     *  @sa m_cdcHitIndicesPositive
     */
    std::set< std::pair < unsigned short, short> > m_cdcHitIndicesNegative;

    /** Name of array of SVDHits (true hits, or clusters) to be accessed. */
    std::string m_svdHitsName;

    /** Pointer to the hitSorterVXD.
     *
     *  This is potentially not good for streaming, as we can in principle point
     *  to a non-ROOTified object.
     *  FIXME Use this for the sorting of the VXDHit indices.
     */
    SorterVXDHits m_hitSorterVXD; //! Don't stream this pointer to file.

    /** SVD indices of the positive arm of the track. */
    std::set<unsigned short> m_svdHitIndicesPositive;

    /** SVD indices of the negative arm of the track. */
    std::set<unsigned short> m_svdHitIndicesNegative;

    /** Name of array of PXDHits (true hits or clusters) to be accessed. */
    std::string m_pxdHitsName;

    /** PXD indices of the positive arm of the track. */
    std::set<unsigned short> m_pxdHitIndicesPositive;

    /** PXD indices of the negative arm of the track. */
    std::set<unsigned short> m_pxdHitIndicesNegative;

    //-----------------------------------------------------------------------------------
    /** Making this class a ROOT class.*/
    ClassDef(RecoTrack, 1);
  };
}

