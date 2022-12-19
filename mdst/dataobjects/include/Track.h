/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <framework/gearbox/Const.h>
#include <framework/datastore/RelationsObject.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <algorithm>

namespace Belle2 {
  /** Class that bundles various TrackFitResults.
   *
   *  As the RecoTrack has a lot of information that is only necessary during the fit,
   *  this is a much stripped down version containing only indices of TrackFitResults
   *  (which is what remains of the various TrackReps, that Genfit might use during the fit).
   */
  class Track : public RelationsObject {
  public:

    /** status enumerator */
    enum StatusBit {

      c_isFlippedAndRefitted = 1 << 0

    };

    /**
    * Pair to hold the particle hypothesis used for the fit as first entry and
    * the result of the track fit as second.
    */
    typedef std::pair< Const::ChargedStable, TrackFitResult*> ChargedStableTrackFitResultPair;

    /** Constructor of Track without any attached TrackFitResults.
     *
     *  Corresponding TrackFitResults should be added using the corresponding setter function.
     *  The array with the indices for the TrackFitResults is initialized with -1,
     *  which is an invalid index.
     *  @param qualityIndicator   Initialises m_qualityIndicator with 0, a value associated typically with fake tracks.
     */
    explicit Track(float qualityIndicator = 0.) : m_qualityIndicator(qualityIndicator)
    {
      std::fill(m_trackFitIndices, m_trackFitIndices + Const::chargedStableSet.size(), -1);
    }

    /** Access to TrackFitResults with a specified Name
     *
     * This tries to return the TrackFitResult for the requested track hypothesis. If the requested track hypothesis is
     * not available, we return a nullptr.
     *
     * TODO: Do something special if we did not even try to fit!
     *
     *  @param chargedStable   Determines the particle for which you want to get the best available fit hypothesis.
     *  @param trackFitResultsName   The name of the storeArray to get the TrackFitResults from
     *  @return TrackFitResult for fit with particle hypothesis given by ParticleCode or a nullptr, if no result is
     *          available.
     *  @sa TrackFitResult
     */
    const TrackFitResult* getTrackFitResultByName(const Const::ChargedStable& chargedStable,
                                                  const std::string trackFitResultsName) const;

    /** Default Access to TrackFitResults.
     *
     * This tries to return the TrackFitResult for the requested track hypothesis. If the requested track hypothesis is
     * not available, we return a nullptr.
     *
     * TODO: Do something special if we did not even try to fit!
     *
     *  @param chargedStable   Determines the particle for which you want to get the best available fit hypothesis.
     *  @return TrackFitResult for fit with particle hypothesis given by ParticleCode or a nullptr, if no result is
     *          available.
     *  @sa TrackFitResult
     */
    const TrackFitResult* getTrackFitResult(const Const::ChargedStable& chargedStable) const;


    /** Return the track fit (from TrackFitResult with specified name) for a fit hypothesis with the closest mass
     *
     * Multiple particle hypothesis are used for fitting during the reconstruction and stored with
     * this Track class. Not all hypothesis are available for all tracks because either a specific hypothesis
     * was not fitted or because the fit failed.
     * This method returns the track fit result of a successful fit with the hypothesis of a mass closest
     * to the requested particle type. If the requested type's hypothesis is available it will be returned
     * otherwise the next closest hypothesis in terms of the absolute mass difference will be returned.
     *
     * For example, if a pion is requested (mass 140 MeV) and only a muon fit (mass 106 MeV) and an
     * electron fit (mass 511 kEV) is available, the muon fit result will be returned.
     * So this method is guaranteed to always return a TrackFitResult (opposite to getTrackFitResult()
     * which can return nullptr if the requested Particle type was not fitted).
     *
     * @param requestedType The particle type for which the fit result should be returned.
     * @param trackFitResultsName  The name of the storeArray to get the TrackFitResults from
     * @return a pointer to the TrackFitResult object. Use TrackFitResult::getParticleType()
     *         to check which fitting hypothesis was actually used for this result.
     */
    const TrackFitResult* getTrackFitResultWithClosestMassByName(const Const::ChargedStable& requestedType,
        const std::string trackFitResultsName) const;

    /** Return the track fit for a fit hypothesis with the closest mass
     *
     * Multiple particle hypothesis are used for fitting during the reconstruction and stored with
     * this Track class. Not all hypothesis are available for all tracks because either a specific hypothesis
     * was not fitted or because the fit failed.
     * This method returns the track fit result of a successful fit with the hypothesis of a mass closest
     * to the requested particle type. If the requested type's hypothesis is available it will be returned
     * otherwise the next closest hypothesis in terms of the absolute mass difference will be returned.
     *
     * For example, if a pion is requested (mass 140 MeV) and only a muon fit (mass 106 MeV) and an
     * electron fit (mass 511 kEV) is available, the muon fit result will be returned.
     * So this method is guaranteed to always return a TrackFitResult (opposite to getTrackFitResult()
     * which can return nullptr if the requested Particle type was not fitted).
     *
     * @param requestedType The particle type for which the fit result should be returned.
     * @return a pointer to the TrackFitResult object. Use TrackFitResult::getParticleType()
     *         to check which fitting hypothesis was actually used for this result.
     */
    const TrackFitResult* getTrackFitResultWithClosestMass(const Const::ChargedStable& requestedType) const;

    /** Access to all track fit results at the same time (from TrackFitResult with specified name)
     *
     * @param trackFitResultsName   The name of the storeArray to get the TrackFitResults from
     * Returns a vector of pair of all track fit results which have been set and the respective particle
     * hypothesis they have been fitted with.
     */
    std::vector<ChargedStableTrackFitResultPair> getTrackFitResultsByName(const std::string trackFitResultsName) const;

    /** Deafult Access to all track fit results at the same time
        *
        * Returns a vector of pair of all track fit results which have been set and the respective particle
        * hypothesis they have been fitted with.
        */
    std::vector<ChargedStableTrackFitResultPair> getTrackFitResults() const;

    /** Add Track Refining Status Bit
     * @param bitmask to be added to the m_statusBitmap
     */
    void addStatusBits(unsigned short int bitmask) { m_statusBitmap |= bitmask; }

    /**Set the Track status bit when the track
     * has been flipped and refitted in the refining step
     */
    void setFlippedAndRefitted() { addStatusBits(c_isFlippedAndRefitted); }

    /** Get Track Status after Refining
     * @param bitmask
     * @return status (1 or 0) corresponding to the bitmask
     */
    bool getStatusBit(unsigned short int bitmask) const { return (m_statusBitmap & bitmask) == bitmask; }

    /** Check the Track status after the Refining step
     * @return true if the track was flipped and refitted in the refining step
     */
    bool isFlippedAndRefitted() const { return getStatusBit(c_isFlippedAndRefitted); }

    /** Check whether Track was modified in the Refining step
     * @return true if the track was modified in the refining step
     */
    bool wasRefined() { return m_statusBitmap > 0; }

    /** Set an index (for positive values) or unavailability-code (index = -1) for a specific mass hypothesis.
      *
      *  The TrackFitResult itself should be saved separately in the DataStore.
      *
      *  @param chargedStable  Determines the hypothesis for which you want to store the index or unavailability-code.
      *  @param index  index of track fit result (for positive values) or unavailability-code (index = -1)
      */
    void setTrackFitResultIndex(const Const::ChargedStable& chargedStable, short index)
    {
      m_trackFitIndices[chargedStable.getIndex()] = index;
    }

    /** Returns the number of fitted hypothesis which are stored in this track. */
    unsigned int getNumberOfFittedHypotheses() const;

    /** Getter for quality indicator for classification of fake vs. MC-matched Tracks.
     *
     *  During reconstruction, the probability (given a certain sample composition) of a track
     *  to originate from a charged particle rather than e.g. a random combination of hits from
     *  different charged particles and background contributions is estimated. This estimate
     *  includes information, that isn't used for the calculation of the p-value of the fit, e.g.
     *  energy-deposition, timing, and cluster-shape information.
     *  We consider it unlikely, that we will make such an estimate for each hypothesis. Therfore,
     *  the Track rather than the TrackFitResult is the place to store this information.
     *  We don't want to provide a default cut, because charged-particle-vetos and
     *  recombination of different kind of resonances potentially can profit from different
     *  working points on the efficiency-purity curve, and we expect some MC-data discrepancy due
     *  to imperfect calibration of the local reconstruction inputs.
     *  The meaning of the value may strongly depend on the presence of VXD or CDC measurements,
     *  but this information is available at mdst level as well.
     */
    float getQualityIndicator() const
    {
      return m_qualityIndicator;
    }

    /** Set track time computed as the difference between the outgoing/ingoing arm time (computed with SVD hits) and the SVD EventT0.
     * If both outgoing and ingoing arms exist:
     * 1) if the outgoing arm time is smaller than the ingoing arm time, the track time is computed as the difference of the outgoing arm time and the SVD EventT0;
     * 2) otherwise the track time is computed as the difference of the ingoing arm time and the SVD EventT0.
     * If only the outgoing arm exists, the track time is computed as the difference of the outgoing arm time and the SVD EventT0.
     * If only the ingoing arm exists, the track time is computed as the difference of the ingoing arm time and the SVD EventT0. */
    void setTrackTime(float track_time)
    {
      m_trackTime = track_time;
    }

    /** Returns track time computed as the difference between the average of SVD clusters time and the SVD EventT0 */
    float getTrackTime() const
    {
      return m_trackTime;
    }

    /** Return a short summary of this object's contents in HTML format. */
    virtual std::string getInfoHTML() const override;

  private:

    /** Bitmap of the track status, contains informations on the refining stage
    */
    unsigned short int m_statusBitmap = 0;

    /** Index list of the TrackFitResults associated with this Track. */
    short int m_trackFitIndices[Const::ChargedStable::c_SetSize];

    /**
     * Returns a vector of all fit hypothesis indices in m_trackFitIndices
     * which have been set (meaning are not -1)
     */
    std::vector < short int > getValidIndices() const;

    /** Quality indicator for classification of fake vs. MC-matched Tracks.
     *
     *  Given likely data-MC discrepancies etc orders of magnitude larger than float precision,
     *  single precision is enough.
     */
    float const m_qualityIndicator;

    /** Track time, computed as the difference between outgoing/ingoing arm time and the SVDEvent T0 */
    float m_trackTime = std::numeric_limits<float>::quiet_NaN();

    ClassDefOverride(Track, 6); /**< Class that bundles various TrackFitResults. */

    friend class FixMergedObjectsModule;
  };
}
