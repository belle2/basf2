/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefano Spataro                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef FILLTRACKFITNTUPLEMODULE_H_
#define FILLTRACKFITNTUPLEMODULE_H_

#include <framework/core/Module.h>
#include <tracking/modules/trackingPerformanceEvaluation/PerformanceEvaluationBaseClass.h>

#include <TTree.h>
#include <TFile.h>
#include <TList.h>
#include <TNtuple.h>

#include <mdst/dataobjects/MCParticle.h>
#include <tracking/dataobjects/MCParticleInfo.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
// forward declarations
namespace Belle2 {
  class Track;
  class TrackFitResult;

  template< class T >
  class StoreArray;


  /** This module takes the Tracks and the RecoTrack
   *  input and produce a root file containing an nutple
   *  showing the track fit members in parallel for different particle hypotheses.
   */
  class FillTrackFitNtupleModule : public Module, PerformanceEvaluationBaseClass {

  public:

    FillTrackFitNtupleModule();

    virtual ~FillTrackFitNtupleModule();

    /// Require the store arrays and create the output root file
    virtual void initialize() override;

    /// Loop over Track objects and fill ntuples with tracking parameters
    virtual void event() override;

    /// Save output root file with ntuple
    virtual void terminate() override;

  private:

    /* user-defined parameters */
    std::string m_RecoTracksName; /**< RecoTrack StoreArray name */
    std::string m_TracksName; /**< Track StoreArray name */
    int m_ParticleHypothesis;  /**< Particle Hypothesis for the track fit (default: 211) */

    //Ntuple for multi hypothesis particle
    TNtuple* m_n_MultiParticle = nullptr;  /**< Multi particle ntuple*/

    StoreArray<RecoTrack> m_RecoTracks; /**< RecoTrack StoreArray */
    StoreArray<Track> m_Tracks; /**< Track StoreArray */
  };
} // end of namespace


#endif /* FILLTRACKFITNTUPLEMODULE_H_ */
