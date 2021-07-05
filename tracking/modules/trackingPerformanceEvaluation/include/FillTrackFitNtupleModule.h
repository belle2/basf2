/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <tracking/modules/trackingPerformanceEvaluation/PerformanceEvaluationBaseClass.h>

#include <TNtuple.h>

#include <framework/datastore/StoreArray.h>

#include <mdst/dataobjects/Track.h>

#include <tracking/dataobjects/RecoTrack.h>

// forward declarations
namespace Belle2 {

  /** This module takes the Tracks and the RecoTrack
   *  input and produce a root file containing an nutple
   *  showing the track fit members in parallel for different particle hypotheses.
   */
  class FillTrackFitNtupleModule : public Module, PerformanceEvaluationBaseClass {

  public:

    /// Default Empty Constructor
    FillTrackFitNtupleModule();

    /// Require the store arrays and create the output root file
    void initialize() override;

    /// Loop over Track objects and fill ntuples with tracking parameters
    void event() override;

    /// Save output root file with ntuple
    void terminate() override;

  private:

    /* user-defined parameters */
    std::string m_RecoTracksName; /**< RecoTrack StoreArray name */
    std::string m_TracksName; /**< Track StoreArray name */
    int m_ParticleHypothesis;  /**< Particle Hypothesis for the track fit (default: 211) */

    TNtuple* m_n_MultiParticle = nullptr;  /**< Multi particle ntuple*/

    StoreArray<RecoTrack> m_RecoTracks; /**< RecoTrack StoreArray */
    StoreArray<Track> m_Tracks; /**< Track StoreArray */
  };
} // end of namespace
