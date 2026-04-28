/**************************************************************************
* basf2 (Belle II Analysis Software Framework)                            *
* Author: The Belle II Collaboration                                      *
*                                                                         *
* See git log for contributors and copyright holders.                     *
* This file is licensed under LGPL-3.0, see LICENSE.md.                   *
***************************************************************************/
#pragma once

#include <TMath.h>
#include <TObject.h>

namespace Belle2 {

  /** The payload containing all parameters for the SVD and CDC CKF.
   *
   * This payload stores scalar, string, and filter parameters used by the ToCDCCKF algorithm.
   * It is designed to be ROOT-serializable for storage in the Conditions Database.
   *
   * The payload includes:
   * - Scalar parameters (float, int, unsigned int, bool) for algorithm configuration
   * - String parameters for store array names, filter names, and directions
   * - Filter parameter maps separated by data type to ensure ROOT compatibility
   *
   * Usage:
   * - Load via DBObjPtr<SVDToCDCCKFParameters> in CKFToCDCFindlet::beginRun()
   * - Apply parameters to sub-findlets using the provided setters
   */

  class SVDToCDCCKFParameters : public TObject {

  public:
    /** Default constructor
     *
     *  Initializes all parameters to their default values matching the ToCDCCKF module defaults.
     */
    SVDToCDCCKFParameters()
      : m_maximalDeltaPhi(TMath::Pi() / 8),
        m_minimalPtRequirement(0.0f),
        m_maximalLayerJump(2),
        m_maximalLayerJumpBackwardSeed(3),
        m_pathMaximalCandidatesInFlight(3),
        m_stateMaximalHitCandidates(4)
    {}

    /** Destructor */
    virtual ~SVDToCDCCKFParameters() {}


    /** FLOAT PARAMETERS
     *
     * Getter & setter methods for float parameters
     */

    /** Set maximal delta phi for CKF state creation
     *
     *  @param phi Maximal delta phi in radians
     */
    void setMaximalDeltaPhi(float phi);

    /** Get maximal delta phi for CKF state creation
     *
     *  @return Maximal delta phi in radians
     */
    float getMaximalDeltaPhi() const;

    /** Set minimal pT requirement for tracks
     *
     *  @param pt Minimal pT in GeV/c
     */
    void setMinimalPtRequirement(float pt);

    /** Get minimal pT requirement for tracks
     *
     *  @return Minimal pT in GeV/c
     */
    float getMinimalPtRequirement() const;


    /** INT PARAMETERS
     *
     * Getter & setter methods for int parameters
     */

    /** Set maximal layer jump for CKF state creation
     *
     *  @param layer Maximal number of layers to jump
     */
    void setMaximalLayerJump(int layer);

    /** Get maximal layer jump for CKF state creation
     *
     *  @return Maximal number of layers to jump
     */
    int getMaximalLayerJump() const;

    /** Set maximal layer jump for backward seed tracks
     *
     *  @param layer Maximal number of layers to jump for backward seeds
     */
    void setMaximalLayerJumpBackwardSeed(int layer);

    /** Get maximal layer jump for backward seed tracks
     *
     *  @return Maximal number of layers to jump for backward seeds
     */
    int getMaximalLayerJumpBackwardSeed() const;

    /** Set maximal number of candidates in flight for path selection
     *
     *  @param max Maximum number of candidates in flight
     */
    void setPathMaximalCandidatesInFlight(unsigned int max);

    /** Get maximal number of candidates in flight for path selection
     *
     *  @return Maximum number of candidates in flight
     */
    unsigned int getPathMaximalCandidatesInFlight() const;

    /** Set maximal number of hit candidates for state filtering
     *
     *  @param max Maximum number of hit candidates
     */
    void setStateMaximalHitCandidates(unsigned int max);

    /** Get maximal number of hit candidates for state filtering
     *
     *  @return Maximum number of hit candidates
     */
    unsigned int getStateMaximalHitCandidates() const;


  private:
    /** FLOAT PARAMETERS
     *
     * Set of Float variables (4 bytes).
     */

    /** Maximal distance in phi between wires for Z=0 plane (radians) */
    float m_maximalDeltaPhi;

    /** Minimal pT requirement for input tracks (GeV/c) */
    float m_minimalPtRequirement;

    /** INTEGER PARAMETERS
     *
     * Set of Integer variables (4 bytes) — layer indices are discrete, so int not float.
     */

    /** Maximal jump over N layers */
    int m_maximalLayerJump;

    /** Maximal jump over N layers for backward seed tracks */
    int m_maximalLayerJumpBackwardSeed;

    /** Maximal number of candidates in flight for path selection */
    unsigned int m_pathMaximalCandidatesInFlight;

    /** Maximal number of hit candidates for state filtering */
    unsigned int m_stateMaximalHitCandidates;

    ClassDef(SVDToCDCCKFParameters, 1);  /**< ClassDef, necessary for ROOT */
  };
}

