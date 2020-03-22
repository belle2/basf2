/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Li Gioi, Thibaud Humair                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

// DataStore
#include <framework/database/DBObjPtr.h>

// rave
#include <analysis/VertexFitting/RaveInterface/RaveSetup.h>
#include <analysis/VertexFitting/RaveInterface/RaveVertexFitter.h>
#include <analysis/VertexFitting/RaveInterface/RaveKinematicVertexFitter.h>

// kFitter
#include <CLHEP/Geometry/Point3D.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Vector/LorentzVector.h>

// DataObjects
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <analysis/dataobjects/FlavorTaggerInfo.h>
#include <mdst/dbobjects/BeamSpot.h>
#include <mdst/dataobjects/MCParticle.h>

#include <string>
#include <vector>


namespace Belle2 {

  class Particle;
  struct TrackAndWeight;

  /**
   * Tag side Vertex Fitter module for modular analysis
   *
   * This module fits the Btag Vertex    *
   */
  class TagVertexModule : public Module {

  public:

    /**
     * Constructor
     */
    TagVertexModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     * Set run dependent things
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    /**
     * returns the BeamSpot object
     */
    const BeamSpot& getBeamSpot() const { return *m_beamSpotDB; }


  private:

    //std::string m_EventType;      /**< Btag decay type */
    std::string m_listName;         /**< Breco particle list name */
    double m_confidenceLevel;       /**< required fit confidence level */
    std::string m_useMCassociation; /**< No MC assication or standard Breco particle or internal MCparticle association */
    //std::string m_useFitAlgorithm;    /**< Choose constraint: from Breco or tube in the boost direction w/wo cut */
    std::string m_constraintType;   /**< Choose constraint: noConstraint, IP, tube, boost, (breco) */
    std::string m_trackFindingType;   /**< Choose how to find the tag tracks: standard, standard_PXD, singleTrack, singleTrack_PXD */
    int m_reqPXDHits;                /**< N of PXD hits for a track to be used */
    std::string m_roeMaskName;      /**< ROE particles from this mask will be used for vertex fitting */
    double m_Bfield;              /**< magnetic field from data base */
    std::vector<const Track*> m_tagTracks;  /**< tracks of the rest of the event */
    std::vector<const TrackFitResult*> m_raveTracks; /**< tracks given to rave for the track fit (after removing Kshorts */
    std::vector<double> m_raveWeights; /**< Store the weights used by Rave in the vtx fit so that they can be accessed later */
    std::vector<const MCParticle*>
    m_raveTracksMCParticles; /**< Store the MC particles corresponding to each track used by Rave in the vtx fit */
    //std::vector<int> m_raveTracksMatchStatus; /**< stor the MC match status related to each track used by rave:
    //                                           * 0: no match, 1: match to good track (direct from tag B) 2: match to bad track */
    double m_fitPval;             /**< P value of the tag side fit result */
    TVector3 m_tagV;              /**< tag side fit result */
    TMatrixDSym m_tagVErrMatrix;  /**< Error matrix of the tag side fit result */
    TVector3 m_MCtagV;            /**< generated tag side vertex */
    int m_mcPDG;                  /**< generated tag side B flavor */
    TVector3 m_MCVertReco;        /**< generated Breco decay vertex */
    double m_deltaT;              /**< reconstructed DeltaT */
    double m_deltaTErr;           /**< reconstructed DeltaT error */
    double m_MCdeltaT;            /**< generated DeltaT */
    double m_MCdeltaTapprox;      /**< generated DeltaT with z-coordinate approximation */
    TMatrixDSym m_constraintCov;  /**< constraint to be used in the tag vertex fit */
    TVector3 m_constraintCenter;  /**< centre position of the constraint for the tag Vertex fit */
    TVector3 m_BeamSpotCenter;    /**< Beam spot position */
    TMatrixDSym m_BeamSpotCov;    /**< size of the beam spot == covariance matrix on the beam spot position */
    bool m_MCInfo;                /**< true if user wants to retrieve MC information out from the tracks used in the fit */
    double m_shiftZ;              /**< parameter for testing the systematic error from the IP measurement*/
    DBObjPtr<BeamSpot> m_beamSpotDB;/**< Beam spot database object*/
    int m_FitType;                /**< fit algo used  */
    double m_tagVl;               /**< tagV component in the boost direction  */
    double m_truthTagVl;          /**< MC tagV component in the boost direction  */
    double m_tagVlErr;            /**< Error of the tagV component in the boost direction  */
    double m_tagVol;              /**< tagV component in the direction orthogonal to the boost */
    double m_truthTagVol;         /**< MC tagV component in the direction orthogonal to the boost */
    double m_tagVolErr;           /**< Error of the tagV component in the direction orthogonal to the boost */
    double m_tagVNDF;             /**< Number of degrees of freedom in the tag vertex fit */
    double m_tagVChi2;            /**< chi^2 value of the tag vertex fit result */
    double m_tagVChi2IP;          /**< IP component of the chi^2 of the tag vertex fit result */
    std::string m_fitAlgo;        /**< Algorithm used for the tag fit (Rave or KFit) */
    bool m_verbose;               /**< choose if you want to print extra infos */
    TMatrixDSym m_pvCov;          /**< covariance matrix of the PV (useful with tube and kfitter) */
    TLorentzVector m_tagMomentum; /**< B tag momentum computed from fully reconstructed B sig */


    /** central method for the tag side vertex fit */
    bool doVertexFit(Particle* Breco);

    /** find intersection between B rec and beam spot (= origin of BTube) */
    bool doVertexFitForBTube(Particle* mother);

    /** calculate the constraint for the vertex fit on the tag side using Breco information*/
    bool findConstraint(Particle* Breco, double cut);

    /** calculate the standard constraint for the vertex fit on the tag side*/
    bool findConstraintBoost(double cut, double shiftAlongBoost = -2000.);

    /** calculate constraint for the vertex fit on the tag side using the B tube (cylinder along
    the expected BTag line of flights */
    bool findConstraintBTube(Particle* Breco, double cut);

    /** get the vertex of the MC B particle associated to Btag. It works anly with signal MC */
    void BtagMCVertex(Particle* Breco);

    /** compare Breco with the two MC B particles */
    bool compBrecoBgen(Particle* Breco, MCParticle* Bgen);

    /** asks for the MC information of the tracks performing the vertex fit *
     *  returns a list of pointers to the MC particles and their kinds, ie
     *  1 for good particle coming directly from the tag B and 2 otherwise
     */
    std::vector< std::pair<const MCParticle*, int> > FlavorTaggerInfoMCMatch(Particle* Breco);


    /** performs the fit using the standard algorithm - using all tracks in RoE
    The user can specify a request on the PXD hits left by the tracks*/
    bool getTagTracks_standardAlgorithm(Particle* Breco, int nPXDHits);

    /** performs the vertex fit using only one track
    The user can specify a request on the PXD hits left by the tracks*/
    bool getTagTracks_singleTrackAlgorithm(Particle* Breco, int nPXDHits);

    /** eliminates an invalid track from a track list **/
    void eliminateTrack(std::vector<int>& listTracks, int trackPosition);

    /** Get a list of track results from a list of tracks removing the Kshorts **/
    bool getTracksWithoutKS(std::vector<const Track*> const&  tagTracks, std::vector<TrackAndWeight>& trackAndWeights);

    /** TO DO: tag side vertex fit in the case of semileptonic tag side decay */
    //bool makeSemileptonicFit(Particle *Breco);


    /**
     * make the vertex fit on the tag side with chosen fit algorithm
     */
    bool makeGeneralFit();

    /**
     * make the vertex fit on the tag side:
     * RAVE AVF
     * tracks coming from Ks removed
     * all other tracks used
     */
    bool makeGeneralFitRave();

    /**
     * make the vertex fit on the tag side:
     * KFit
     * tracks coming from Ks removed
     * all other tracks used
     */
    bool makeGeneralFitKFit();

    /**
     * calculate DeltaT and MC-DeltaT (rec - tag) in ps from Breco and Btag vertices
     * DT = Dl / gamma beta c  ,  l = boost direction
     */
    void deltaT(Particle* Breco);

    /**
     * Print a TVector3 (useful for debugging)
     */
    std::string printVector(TVector3 const& vec);

    /**
     * Print a TMatrix (useful for debugging)
     */
    std::string printMatrix(TMatrix const& mat);

    /**
     * Print a TMatrixFSym (useful for debugging)
     */
    std::string printMatrix(TMatrixFSym const& mat);
  };

  /**
   * this struct is used to store and sort the tag tracks
   */
  struct TrackAndWeight {
    double weight; /**< rave weight associated to the track, for sorting purposes*/
    const TrackFitResult* track;  /**< tag track fit result, for sorting purposes */
    const MCParticle* mcParticle;  /**< mc particle matched to the tag track, for sorting purposes*/
  };

  /**
   * This is used to sort the tag tracks by rave weight, to have the tracks having the
   * most significance contribution to the tag vertex fit appearing first
   */
  bool compare(TrackAndWeight& a, TrackAndWeight& b) { return a.weight > b.weight; }
}
