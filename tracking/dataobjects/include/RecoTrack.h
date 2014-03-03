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

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>

#include <genfit/Track.h>

#include <set>
#include <vector>
#include <string>

/** This is the Reconstruction Event-Data Model Track.
 *
 *  This class collects hits, performs fits and saves the Track parameters.
 *  Note: This class is still experimental.
 */
namespace Belle2 {
  class RecoTrack : public Belle2::RelationsInterface <genfit::Track> {
  public:
    RecoTrack(std::string cdcHitsName = "", std::string svdHitsName = "", std::string pxdHitsName = "") :
      m_cdcHitsName(cdcHitsName), m_svdHitsName(svdHitsName), m_pxdHitsName(pxdHitsName)
    {}

    //-------------------------------------- CDC Part -----------------------------------
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

    //-------------------------------------- SVD Part -----------------------------------
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

    //-------------------------------------- PXD Part -----------------------------------
    //-----------------------------------------------------------------------------------
    /** Replacing the existing PXD indices with new indices.
     *
     *  @param pseudocharge  Shall those indices be used for the positive (or negative)
     *                       arm of the track.
     */
    void setPXDHitIndices(const std::set< unsigned short >& pxdHitIndices,
                          const short pseudocharge) {
      pseudocharge > 0 ?
      (m_svdHitIndicesPositive = pxdHitIndices) : (m_svdHitIndicesNegative = pxdHitIndices);
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

  private:

    //needs some Comparator class, that makes sure, positively charged

    //-------------------------------------- Hit Indices Storage ------------------------
    //-----------------------------------------------------------------------------------
    /** Name of array of CDCHits to be accessed. */
    std::string m_cdcHitsName;

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
    /** Name of array of PXDHits (true hits or clusters) to be accessed. */
    std::string m_pxdHitsName;
    * /
    std::set< std::pair < unsigned short, short> > m_cdcHitIndicesNegative;

    /** Name of array of SVDHits (true hits, or clusters) to be accessed. */
    std::string m_svdHitsName;

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

