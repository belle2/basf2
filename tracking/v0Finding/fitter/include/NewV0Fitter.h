/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/V0.h>
#include <tracking/dataobjects/V0ValidationVertex.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <genfit/Track.h>
#include <genfit/GFRaveVertex.h>
#include <map>

namespace Belle2 {

  /**
   * Improved V0 fitter class
   */
  class NewV0Fitter {

  public:

    /**
     * Structure to save track data of the last successful iteration
     */
    struct FittedTrack {
      const RecoTrack* recoTrack = nullptr; /**< reco track */
      Const::ParticleType ptype = Const::invalidParticle; /**< particle type of the V0 track */
      genfit::MeasuredStateOnPlane state; /**< measured state at first hit, extrapolated to fitted vertex */
      double pValue = 0; /**< p-value of track fit */
      int Ndf = 0; /**< degrees-of-freedom of track fit */

      /**
       * Sets the data members
       * @param recoTrk reco track
       * @param hypo particle type of the V0 track (not necessary the one used for the track fit!)
       * @param mSoP measured state at first hit extrapolated to fitted vertex
       * @param rep track representation used for track fit
       */
      void set(const RecoTrack* recoTrk, const Const::ParticleType& hypo, const genfit::MeasuredStateOnPlane& mSoP,
               const genfit::AbsTrackRep* rep)
      {
        recoTrack = recoTrk;
        ptype = hypo;
        state = mSoP;
        const auto* fitStatus = recoTrk->getGenfitTrack().getFitStatus(rep);
        pValue = fitStatus->getPVal();
        Ndf = fitStatus->getNdf();
      }
    };


    /**
     * Constructor
     * @param trackFitResultsName name of the StoreArray TrackFitResults
     * @param v0sName name of the StoreArray V0s
     * @param v0ValidationVerticesName name of the StoreArray V0ValidationVertex
     * @param recoTracksName name of the StoreArray RecoTracks
     * @param copiedRecoTracksName name of the StoreArray of copied RecoTracks
     * @param enableValidation on true store additional data for validation
     */
    NewV0Fitter(const std::string& trackFitResultsName = "",
                const std::string& v0sName = "",
                const std::string& v0ValidationVerticesName = "",
                const std::string& recoTracksName = "",
                const std::string& copiedRecoTracksName = "CopiedRecoTracks",
                bool enableValidation = false);

    /**
     * Initialization of cuts applied during the fit and store process.
     * @param vertexDistanceCut cut on the transverse radius to cut-off vertices within the beam pipe
     * @param vertexChi2Cut cut on the vertex chi^2
     * @param invMassRangeKshort selection mass window for Ks
     * @param invMassRangeLambda selection mass window for Lambda
     * @param invMassRangePhoton selection mass window for converted gamma
     */
    void initializeCuts(double vertexDistanceCut,
                        double vertexChi2Cut,
                        const std::tuple<double, double>& invMassRangeKshort,
                        const std::tuple<double, double>& invMassRangeLambda,
                        const std::tuple<double, double>& invMassRangePhoton);

    /**
     * Setter for the fitter mode.
     *   0: store V0 at the first vertex fit, regardless of inner hits
     *   1: remove hits inside the V0 vertex position (default)
     *   2: mode 1 +  don't use SVD hits if there is only one available SVD hit-pair
     * @param fitterMode fitter mode
     */
    void setFitterMode(int fitterMode) {m_fitterMode = fitterMode;}

    /**
     * Fit V0 with given hypothesis and store results if fit is successful.
     * @param trackPlus positively charged track
     * @param trackMinus negatively charged track
     * @param v0Hypothesis V0 hypothesis
     * @param isForceStored true if V0 is forced to store when hit removal failed [out]
     * @param isHitRemoved true if inner hits have been removed (or at least tried to be removed) [out]
     * @return true on success
     */
    bool fitAndStore(const Track* trackPlus, const Track* trackMinus, const Const::ParticleType& v0Hypothesis,
                     bool& isForceStored, bool& isHitRemoved);

    /**
     * Returns daughter particle types for a given V0 hypothesis.
     * @param v0Hypothesis V0 hypothesis
     * @return a pair of daughter particle types
     */
    static std::pair<Const::ParticleType, Const::ParticleType> getTrackHypotheses(const Const::ParticleType& v0Hypothesis);


  private:

    /**
     * Error status codes returned by the vertexFit method
     */
    enum ErrorStatus {
      c_NoTrackRepresentation = -4, /**< no track representation for given PDG code */
      c_VertexFitFailed = -3,       /**< vertex fit failed */
      c_ExtrapolationFailed = -2,   /**< track extrapolation failed */
      c_NotSelected = -1            /**< fitted vertex not passing the cuts */
    };

    /**
     * Performs a vertex fit. On success the return value indicates if tracks have inner hits or not
     * (bit 0 set for positive, bit 1 set for negative track). On failure it returns ErrorStatus code.
     * @param recoTrackPlus positively charged track
     * @param recoTrackMinus negatively charged track
     * @param pdgTrackPlus PDG code used in the fit of positively charged track (closest mass to V0 daughter)
     * @param pdgTrackMinus PDG code used in the fit of negatively charged track (closest mass to V0 daughter)
     * @param v0Hypothesis V0 hypothesis
     * @return value >= 0 on success, value < 0 on failure
     */
    int vertexFit(const RecoTrack* recoTrackPlus, const RecoTrack* recoTrackMinus,
                  int pdgTrackPlus, int pdgTrackMinus, const Const::ParticleType& v0Hypothesis);

    /**
     * Returns track representation for a given PDG code.
     * @param recoTrack track
     * @param pdgCode PDG code
     * @return track representation on success or nullptr on failure
     */
    const genfit::AbsTrackRep* getTrackRepresentation(const RecoTrack* recoTrack, int pdgCode);

    /**
     * Sets cardinal representation of a given genfit track and PDG code.
     * @param gfTrack genfit track [in/out]
     * @param pdgCode PDG code
     * @return true on success
     */
    bool setCardinalRep(genfit::Track& gfTrack, int pdgCode);

    /**
     * Genfit Rave vertex fit called by vertexFit method.
     * @param trackPlus positively charged genfit track
     * @param trackMinus negatively charged genfit track
     * @param vertex fitted vertex [out]
     * @return true on success
     */
    bool fitGFRaveVertex(genfit::Track& trackPlus, genfit::Track& trackMinus, genfit::GFRaveVertex& vertex);

    /**
     * Extrapolation of both tracks to the vertex. On success the return value indicates if tracks have inner hits
     * (bit 0 set for positive, bit 1 set for negative track).
     * @param statePlus measured state of positively charged track from which the extrapolation is performed [in/out]
     * @param stateMinus measured state of negatively charged track from which the extrapolation is performed [in/out]
     * @param vertex vertex
     * @return value >= 0 on success, value < 0 on failure
     */
    int extrapolateToVertex(genfit::MeasuredStateOnPlane& statePlus, genfit::MeasuredStateOnPlane& stateMinus,
                            const genfit::GFRaveVertex& vertex);

    /**
     * Make a copy of reco track.
     * @param origRecoTrack source
     * @param state measured state for the track fit seeding
     * @return a copy of source
     */
    RecoTrack* copyRecoTrack(const RecoTrack* origRecoTrack, const genfit::MeasuredStateOnPlane& state);

    /**
     * Remove track inner hits and refit the track. On success return the refitted one,
     * otherwise return the track of last iteration (or the original one if no hits need to be removed).
     * @param origRecoTrack original track
     * @param lastRecoTrack track at last hit-removal iteration
     * @param ptype particle type for refit
     * @return refitted track on success, lastRecoTrack on failure, origRecoTrack if no hits removed
     */
    const RecoTrack* removeHitsAndRefit(const RecoTrack* origRecoTrack, const RecoTrack* lastRecoTrack,
                                        const Const::ParticleType& ptype);

    /**
     * Returns bit flags indicating that the innermost cluster is shared between both tracks.
     * Bit 0 is set if U cluster is shared, bit 1 is set if V cluster is shared, and both if shared cluster is PXD.
     * @param recoTrackPlus positively charged track
     * @param recoTrackMinus negatively charged track
     * @return bit flags
     */
    int isInnermostClusterShared(const RecoTrack* recoTrackPlus, const RecoTrack* recoTrackMinus);

    /**
     * Append track fit result to the collection.
     * @param trk track data
     * @param sharedInnermostCluster bit flags of shared innermost clusters
     * @return track fit result
     */
    const TrackFitResult* saveTrackFitResult(const FittedTrack& trk, int sharedInnermostCluster);


    std::string m_recoTracksName;                    /**< name of the RecoTracks collection */
    StoreArray<RecoTrack> m_recoTracks;              /**< RecoTracks collection */
    StoreArray<TrackFitResult> m_trackFitResults;    /**< TrackFitResults collection */
    StoreArray<V0> m_v0s;                            /**< V0s collection */
    StoreArray<V0ValidationVertex> m_validationV0s;  /**< V0ValidationVertex collection (optional) */
    StoreArray<RecoTrack> m_copiedRecoTracks;        /**< copied RecoTracks collection */

    double m_vertexDistanceCut = 0;  /**< cut on the transverse radius */
    double m_vertexChi2Cut = 0;   /**< Chi2 cut */
    std::map<int, std::pair<double, double> > m_invMassCuts; /**< invariant mass cuts, key = abs(PDG) */

    int m_fitterMode = 1;  /**< fitter mode */
    bool m_validation = false; /**< validation flag */

    // temporary storage

    genfit::GFRaveVertex m_fittedVertex; /**< last successfully fitted vertex */
    double m_momentum = 0; /**< momentum of last successfully fitted vertex */
    double m_invMass = 0; /**< invariant mass of last successfully fitted vertex */
    FittedTrack m_trkPlus; /**< positively charged track data of last successfully fitted vertex */
    FittedTrack m_trkMinus; /**< negatively charged track data of last successfully fitted vertex */

  };

}

