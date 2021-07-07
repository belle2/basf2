/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

// framework - DataStore
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// Rave
#include <analysis/VertexFitting/RaveInterface/RaveSetup.h>
#include <analysis/VertexFitting/RaveInterface/RaveVertexFitter.h>
#include <analysis/VertexFitting/RaveInterface/RaveKinematicVertexFitter.h>

// KFit
#include <CLHEP/Geometry/Point3D.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Vector/LorentzVector.h>

// DataObjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/TagVertex.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dbobjects/BeamSpot.h>
#include <mdst/dataobjects/MCParticle.h>

#include <string>
#include <vector>


namespace Belle2 {

  class Particle;
  struct ParticleAndWeight;

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

    StoreArray<TagVertex> m_verArray; /**< StoreArray of TagVertexes */
    StoreArray<MCParticle> m_mcParticles; /**< StoreArray of MCParticles */
    StoreObjPtr<ParticleList> m_plist; /**< input particle list */
    //std::string m_EventType;      /**< Btag decay type */
    std::string m_listName;         /**< Breco particle list name */
    double m_confidenceLevel;       /**< required fit confidence level */
    std::string m_useMCassociation; /**< No MC association or standard Breco particle or internal MCparticle association */
    //std::string m_useFitAlgorithm;    /**< Choose constraint: from Breco or tube in the boost direction w/wo cut */
    std::string m_constraintType;   /**< Choose constraint: noConstraint, IP, tube, boost, (breco) */
    std::string m_trackFindingType;   /**< Choose how to find the tag tracks: standard, standard_PXD */
    int m_reqPXDHits;                /**< N of PXD hits for a track to be used */
    std::string m_roeMaskName;      /**< ROE particles from this mask will be used for vertex fitting */
    double m_Bfield;              /**< magnetic field from data base */
    std::vector<const Particle*> m_tagParticles;  /**< tracks of the rest of the event */
    std::vector<const Particle*> m_raveParticles; /**< tracks given to rave for the track fit (after removing Kshorts */
    std::vector<double> m_raveWeights; /**< Store the weights used by Rave in the vtx fit so that they can be accessed later */
    std::vector<const MCParticle*>
    m_raveMCParticles; /**< Store the MC particles corresponding to each track used by Rave in the vtx fit */
    bool m_useTruthInFit;  /**< Set to true if the tag fit is to be made with the TRUE tag track momentum and position */
    int m_fitTruthStatus; /**< Store info about whether the fit was performed with the truth info
                           * 0 fit performed with measured parameters
                           * 1 fit performed with true parameters
                           * 2 unable to recover truth parameters */
    bool m_useRollBack;  /**< Set to true if the tag fit is to be made with the tag track position rolled back to mother B */
    int m_rollbackStatus; /**< Store info about whether the fit was performed with the rolled back tracks
                           * 0 fit performed with measured parameters
                           * 1 fit performed with rolled back parameters
                           * 2 unable to recover truth parameters */
    double m_fitPval;             /**< P value of the tag side fit result */
    TVector3 m_tagV;              /**< tag side fit result */
    TMatrixDSym m_tagVErrMatrix;  /**< Error matrix of the tag side fit result */
    TVector3 m_mcTagV;            /**< generated tag side vertex */
    double   m_mcTagLifeTime;     /**< generated tag side life time of B-decay */
    int m_mcPDG;                  /**< generated tag side B flavor */
    TVector3 m_mcVertReco;        /**< generated Breco decay vertex */
    double m_mcLifeTimeReco;      /**< generated Breco life time */
    double m_deltaT;              /**< reconstructed DeltaT */
    double m_deltaTErr;           /**< reconstructed DeltaT error */
    double m_mcDeltaTau;            /**< generated DeltaT */
    double m_mcDeltaT;            /**< generated DeltaT with boost-direction approximation */
    TMatrixDSym m_constraintCov;  /**< constraint to be used in the tag vertex fit */
    TVector3 m_constraintCenter;  /**< centre position of the constraint for the tag Vertex fit */
    TVector3 m_BeamSpotCenter;    /**< Beam spot position */
    TMatrixDSym m_BeamSpotCov;    /**< size of the beam spot == covariance matrix on the beam spot position */
    bool m_mcInfo;                /**< true if user wants to retrieve MC information out from the tracks used in the fit */
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
    TMatrixDSym m_pvCov;          /**< covariance matrix of the PV (useful with tube and KFit) */
    TLorentzVector m_tagMomentum; /**< B tag momentum computed from fully reconstructed B sig */


    /** central method for the tag side vertex fit */
    bool doVertexFit(const Particle* Breco);

    /** it returns an intersection between B rec and beam spot (= origin of BTube) */
    Particle* doVertexFitForBTube(const Particle* mother, std::string fitType) const;

    /** calculate the constraint for the vertex fit on the tag side using Breco information*/
    std::pair<TVector3, TMatrixDSym> findConstraint(const Particle* Breco, double cut) const;

    /** calculate the standard constraint for the vertex fit on the tag side*/
    std::pair<TVector3, TMatrixDSym> findConstraintBoost(double cut, double shiftAlongBoost = -2000.) const;

    /** calculate constraint for the vertex fit on the tag side using the B tube (cylinder along
    the expected BTag line of flights */
    std::pair<TVector3, TMatrixDSym> findConstraintBTube(const Particle* Breco, double cut);

    /** get the vertex of the MC B particle associated to Btag. It works anly with signal MC */
    void BtagMCVertex(const Particle* Breco);

    /** compare Breco with the two MC B particles */
    static bool compBrecoBgen(const Particle* Breco, const MCParticle* Bgen);

    /** performs the fit using the standard algorithm - using all tracks in RoE
    The user can specify a request on the PXD hits left by the tracks*/
    std::vector<const Particle*> getTagTracks_standardAlgorithm(const Particle* Breco, int nPXDHits) const;

    /**
     * Get a list of pions from a list of pions removing the Kshorts
     * Warning: this assumes all the particles are pions, which is fine are all the particles
     * are reconstructed as pions in the TagV module.
     */
    std::vector<ParticleAndWeight> getParticlesWithoutKS(const std::vector<const Particle*>& tagParticles,
                                                         double massWindowWidth = 0.01) const;

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
     * Fill sorted list of particles into external variable
    */
    void fillParticles(std::vector<ParticleAndWeight>& particleAndWeights);


    /**
     * Fill tagV vertex info
    */
    void fillTagVinfo(const TVector3& tagVpos, const TMatrixDSym& tagVposErr);

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
    void deltaT(const Particle* Breco);

    /**
     * Reset all parameters that are computed in each event and then used to compute
     * tuple variables
     */
    void resetReturnParams();

    /**
     * Print a TVector3 (useful for debugging)
     */
    static std::string printVector(const TVector3& vec);

    /**
     * Print a TMatrix (useful for debugging)
     */
    static std::string printMatrix(const TMatrixD& mat);

    /**
     * Print a TMatrixFSym (useful for debugging)
     */
    static std::string printMatrix(const TMatrixDSym& mat);

    /**
     * If the fit has to be done with the truth info, Rave is fed with a track
     * where the momentum is replaced by the true momentum and the position replaced
     * by the point on the true particle trajectory closest to the measured track position
     * The function below takes care of that.
     */
    TrackFitResult getTrackWithTrueCoordinates(ParticleAndWeight const& paw) const;

    /**
     * This finds the point on the true particle trajectory closest to the measured track position
     */
    static TVector3 getTruePoca(ParticleAndWeight const& paw);

    /**
     * If the fit has to be done with the rolled back tracks, Rave or KFit is fed with a track where the
     * position of track is shifted by the vector difference of mother B and production point of track
     * from truth info
     * The function below takes care of that.
     */
    TrackFitResult getTrackWithRollBackCoordinates(ParticleAndWeight const& paw);

    /**
     * This shifts the position of tracks by the vector difference of mother B and production point of track from truth info
     */
    TVector3 getRollBackPoca(ParticleAndWeight const& paw);
  };

  /**
   * this struct is used to store and sort the tag tracks
   */
  struct ParticleAndWeight {
    double weight; /**< rave weight associated to the track, for sorting purposes*/
    const Particle* particle;  /**< tag track fit result with pion mass hypo, for sorting purposes */
    const MCParticle* mcParticle;  /**< mc particle matched to the tag track, for sorting purposes*/
  };

}
