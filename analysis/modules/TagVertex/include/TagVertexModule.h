/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Li Gioi                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TAGVERTEXMODULE_H
#define TAGVERTEXMODULE_H

#include <framework/core/Module.h>

// DataStore
#include <framework/database/DBObjPtr.h>

// rave
#include <analysis/raveInterface/RaveSetup.h>
#include <analysis/raveInterface/RaveVertexFitter.h>
#include <analysis/raveInterface/RaveKinematicVertexFitter.h>

// DataObjects
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <analysis/dataobjects/FlavorTaggerInfo.h>
#include <framework/dbobjects/BeamParameters.h>

#include <string>


namespace Belle2 {

  class Particle;

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
     * returns the BeamParameters object
     */
    const BeamParameters& getBeamParameters() const { return *m_beamParams; }


  private:

    //std::string m_EventType;      /**< Btag decay type */
    std::string m_listName;         /**< Breco particle list name */
    double m_confidenceLevel;       /**< required fit confidence level */
    std::string m_useMCassociation; /**< No MC assication or standard Breco particle or internal MCparticle association */
    std::string m_useFitAlgorithm;    /**< Choose constraint: from Breco or tube in the boost direction w/wo cut */
    int m_reqPXDHits;                /**< N of PXD hits for a track to be used */

    double m_Bfield;              /**< magnetic field from data base */
    std::vector<const Track*> m_tagTracks;  /**< tracks of the rest of the event */
    double m_fitPval;             /**< P value of the tag side fit result */
    TVector3 m_tagV;              /**< tag side fit result */
    TMatrixDSym m_tagVErrMatrix;  /**< Error matrix of the tag side fit result */
    TVector3 m_MCtagV;            /**< generated tag side vertex */
    int m_mcPDG;                  /**< generated tag side B flavor */
    TVector3 m_MCVertReco;        /**< generated Breco decay vertex */
    double m_deltaT;              /**< reconstructed DeltaT */
    double m_deltaTErr;           /**< reconstructed DeltaT error */
    double m_MCdeltaT;            /**< generated DeltaT */
    TMatrixDSym m_tube;           /**< constrained to be used in the tag vertex fit */
    TVector3 m_BeamSpotCenter;    /**< Beam spot position */
    bool m_MCInfo;                /**< true if user wants to retrieve MC information out from the tracks used    in the fit */
    double m_shiftZ;              /**< parameter for testing the systematic errror from the IP measurement*/
    DBObjPtr<BeamParameters> m_beamParams; /**< Beam parameters */
    int m_FitType;                /**< fit algo used  */
    double m_tagVl;               /**< tagV component in the boost direction  */
    double m_truthTagVl;          /**< MC tagV component in the boost direction  */
    double m_tagVlErr;            /**< Error of the tagV component in the boost direction  */
    double m_tagVol;              /**< tagV component in the direction orthogonal to the boost */
    double m_truthTagVol;         /**< MC tagV component in the direction orthogonal to the boost */
    double m_tagVolErr;           /**< Error of the tagV component in the direction orthogonal to the boost */

    /** central method for the tag side vertex fit */
    bool doVertexFit(Particle* Breco);

    /** calculate the constraint for the vertex fit on the tag side using Breco information*/
    bool findConstraint(Particle* Breco, double cut);

    /** calculate the standard constraint for the vertex fit on the tag side*/
    bool findConstraintBoost(double cut);

    /** get the vertex of the MC B particle associated to Btag. It works anly with signal MC */
    void BtagMCVertex(Particle* Breco);

    /** compare Breco with the two MC B particles */
    bool compBrecoBgen(Particle* Breco, MCParticle* Bgen);

    /** asks for the MC information of the tracks performing the vertex fit **/
    void FlavorTaggerInfoMCMatch(Particle* Breco);


    /** performs the fit using the standard algorithm - using all tracks in RoE
    The user can specify a request on the PXD hits left by the tracks*/
    bool getTagTracks_standardAlgorithm(Particle* Breco, int nPXDHits);

    /** performs the vertex fit using only one track
    The user can specify a request on the PXD hits left by the tracks*/
    bool getTagTracks_singleTrackAlgorithm(Particle* Breco, int nPXDHits);

    /** eliminates an invalid track from a track list **/
    void eliminateTrack(std::vector<int>& listTracks, int trackPosition);

    /** TO DO: tag side vertex fit in the case of semileptonic tag side decay */
    //bool makeSemileptonicFit(Particle *Breco);

    /**
     * make the vertex fit on the tag side:
     * RAVE AVF
     * tracks coming from Ks removed
     * all other tracks used
     */
    bool makeGeneralFit();

    /**
     * calculate DeltaT and MC-DeltaT (rec - tag) in ps from Breco and Btag vertices
     * DT = Dl / gamma beta c  ,  l = boost direction
     */
    void deltaT(Particle* Breco);
  };
}
#endif /* TAGVERTEXMODULE_H */
