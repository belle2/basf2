/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TRACKINGOUTPUTMODULE_H_
#define TRACKINGOUTPUTMODULE_H_

#include <framework/core/Module.h>

#include "TTree.h"
#include "TFile.h"


namespace Belle2 {

  /** This is an auxiliary output module for tracking.
   *  It uses the MC information, the Tracks from MCFitting and the Tracks from fitting with pattern recognition.
   *  Many different variables describing the fit results are written out to analyze the performance.
   *  The produced tree files are very large and I am also not very familiar with root trees, so there may still be several flaws which can be improved.
   *
   *  @todo: make the module faste and the output files smaller while keeping all the variables in, when somebody has an idea how, do not hesitate to tell me ;)
   */
  class TrackingOutputModule : public Module {

  public:

    /**
     * Constructor of the module.
     *
     * Sets the description of the module.
     */
    TrackingOutputModule();

    /** Destructor of the module. */
    ~TrackingOutputModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.
     */
    void initialize();

    /** Called when entering a new run.
     */
    void beginRun();

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.
     */
    void event();

    /** This method is called if the current run ends.
     */
    void endRun();

    /** End of the event processing. */
    void terminate();

    /** Returns Ids of the Tracks created by the given MCParticle.
     */
    std::vector <int> getTrackIdsForMCId(std::string Tracks, int MCId);

  private:

    TFile * m_file;                              /**< Output Root file  */

    TTree * m_tree;                              /**< Output Root tree */

    std::string m_mcParticlesColName;            /**< MCParticles collection name */

    std::string m_tracksMCColName;               /**< MCTracks (Tracks created byt fitting MC truth) collection name */

    std::string m_tracksPRColName;               /**< PRTracks (Tracks created by fitting pattern reco results) collection name */

    std::string m_gfTracksMCColName;             /**< MCGFTracks (GFTracks created byt fitting MC truth) collection name */

    std::string m_gfTracksPRColName;             /**< PRGFTracks (GFTracks created by fitting pattern reco results) collection name */

    std::string m_outputFileName;                /**< Name of the output file */


    //branches
    int m_nMCPrimary;                            /**< Number of primary MCParticles */

    int m_mcSuccessCounter;                      /**< Counter for successfully fitted MCTracks */
    int m_prSuccessCounter;                      /**< Counter for successfully fitted PRTracks */

    //-----------------------------------------------------------------------
    float m_mcMomentumX;                         /**< Momentum of the MCParticle (x-component) */
    float m_mcMomentumY;                         /**< Momentum of the MCParticle (y-component) */
    float m_mcMomentumZ;                         /**< Momentum of the MCParticle (z-component) */
    float m_mcMomentum;                          /**< Momentum of the MCParticle (absolute value) */
    float m_mcCurv;                              /**< Curvature of the MCParticle (inverse transverse momentum) */
    float m_mcCotTheta;                          /**< Cotangens theta of the MCParticle (pz/pt) */
    int m_mcPDG;                                 /**< PDG of the MCParticle */
    float m_mcPositionX;                         /**< Vertex of the MCParticle (x-component) */
    float m_mcPositionY;                         /**< Vertex of the MCParticle (y-component) */
    float m_mcPositionZ;                         /**< Vertex of the MCParticle (z-component) */

    //-------------------------------------------------------------------------
    int m_nMCFitTracks;                          /** Number of fitted MCTracks */
    float m_mcStartMomentumX[2];                 /** Start values for the MCTrack fit: momentum (x-component) */
    float m_mcStartMomentumY[2];                 /** Start values for the MCTrack fit: momentum (y-component) */
    float m_mcStartMomentumZ[2];                 /** Start values for the MCTrack fit: momentum (z-component) */
    float m_mcStartMomentum[2];                  /** Start values for the MCTrack fit: momentum (absolute value) */
    float m_mcStartCurv[2];                      /** Start values for the MCTrack fit: curvature (inverse transverse momentum) */
    float m_mcStartCotTheta[2];                  /** Start values for the MCTrack fit: cotangens theta (pz/pt) */
    float m_mcStartPositionX[2];                 /** Start values for the MCTrack fit: vertex (x-component) */
    float m_mcStartPositionY[2];                 /** Start values for the MCTrack fit: vertex (y-component) */
    float m_mcStartPositionZ[2];                 /** Start values for the MCTrack fit: vertex (z-component) */

    float m_mcFitMomentumX[2];                   /** Fit result for the MCTrack fit: momentum (x-component) */
    float m_mcFitMomentumY[2];                   /** Fit result for the MCTrack fit: momentum (y-component) */
    float m_mcFitMomentumZ[2];                   /** Fit result for the MCTrack fit: momentum (z-component) */
    float m_mcFitMomentum[2];                    /** Fit result for the MCTrack fit: momentum (absolute value) */
    float m_mcFitCurv[2];                        /** Fit result for the MCTrack fit: curvature (inverse transverse momentum) */
    float m_mcFitCotTheta[2];                    /** Fit result for the MCTrack fit: cotangens theta(pz/pt) */
    float m_mcFitPositionX[2];                   /** Fit result for the MCTrack fit: vertex (x-component) */
    float m_mcFitPositionY[2];                   /** Fit result for the MCTrack fit: vertex (y-component) */
    float m_mcFitPositionZ[2];                   /** Fit result for the MCTrack fit: vertex (z-component) */

    int m_nMCHits[2];                            /** Number of hits in the fitted MCTrack */
    int m_mcFitPDG[2];                           /** PDG value of the fitted MCTrack */
    float m_mcFitChi2[2];                        /** Chi2 value of the MCTrack fit */
    float m_mcFitPValue[2];                      /** Probability value of the MCTrack fit */
    int m_mcSuccessFit[2];                       /** Marker for successful fit of the MCTrack */
    int m_mcSuccessExtrap[2];                    /** Marker of successful extrapolation of the MCTrack fit */

    float m_mcFitMomentumXErr[2];                /** Fit result for the MCTrack fit: momentum error (x-component) */
    float m_mcFitMomentumYErr[2];                /** Fit result for the MCTrack fit: momentum error (y-component) */
    float m_mcFitMomentumZErr[2];                /** Fit result for the MCTrack fit: momentum error (z-component) */
    float m_mcFitPositionXErr[2];                /** Fit result for the MCTrack fit: vertex error (x-component) */
    float m_mcFitPositionYErr[2];                /** Fit result for the MCTrack fit: vertex error (y-component) */
    float m_mcFitPositionZErr[2];                /** Fit result for the MCTrack fit: vertex error (z-component) */

    std::vector <int> m_mcFitTracks;             /** MCTracks Ids for the given MCParticle */

    //---------------------------------------------------------------------------
    int m_nPRFitTracks;                          /** Number of fitted PRTracks */
    float m_prStartMomentumX[20];                 /** Start values for the PRTrack fit: momentum (x-component) */
    float m_prStartMomentumY[20];                 /** Start values for the PRTrack fit: momentum (y-component) */
    float m_prStartMomentumZ[20];                 /** Start values for the PRTrack fit: momentum (z-component) */
    float m_prStartMomentum[20];                  /** Start values for the PRTrack fit: momentum (absolute value) */
    float m_prStartCurv[20];                      /** Start values for the PRTrack fit: curvature (inverse transverse momentum) */
    float m_prStartCotTheta[20];                  /** Start values for the PRTrack fit: cotangens theta (pz/pt) */
    float m_prStartPositionX[20];                 /** Start values for the PRTrack fit: vertex (x-component) */
    float m_prStartPositionY[20];                 /** Start values for the PRTrack fit: vertex (y-component) */
    float m_prStartPositionZ[20];                 /** Start values for the PRTrack fit: vertex (z-component) */

    float m_prFitMomentumX[20];                   /** Fit result for the PRTrack fit: momentum (x-component) */
    float m_prFitMomentumY[20];                   /** Fit result for the PRTrack fit: momentum (y-component) */
    float m_prFitMomentumZ[20];                   /** Fit result for the PRTrack fit: momentum (z-component) */
    float m_prFitMomentum[20];                    /** Fit result for the PRTrack fit: momentum (absolute value) */
    float m_prFitCurv[20];                        /** Fit result for the PRTrack fit: curvature (inverse transverse momentum) */
    float m_prFitCotTheta[20];                    /** Fit result for the PRTrack fit: cotangens theta(pz/pt) */
    float m_prFitPositionX[20];                   /** Fit result for the PRTrack fit: vertex (x-component) */
    float m_prFitPositionY[20];                   /** Fit result for the PRTrack fit: vertex (y-component) */
    float m_prFitPositionZ[20];                   /** Fit result for the PRTrack fit: vertex (z-component) */

    int m_nPRHits[20];                            /** Number of hits in the fitted PRTrack */
    int m_prFitPDG[20];                           /** PDG value of the fitted PRTrack */
    float m_prFitChi2[20];                        /** Chi2 value of the PRTrack fit */
    float m_prFitPValue[20];                      /** Probability value of the PRTrack fit */
    int m_prSuccessFit[20];                       /** Marker for successful fit of the PRTrack */
    int m_prSuccessExtrap[20];                    /** Marker of successful extrapolation of the PRTrack fit */
    float m_prPurity[20];                         /** Purity of the PRTrack ((hits from one MCParticle/total nr of hits) * 100) */

    float m_prFitMomentumXErr[20];                /** Fit result for the PRTrack fit: momentum error (x-component) */
    float m_prFitMomentumYErr[20];                /** Fit result for the PRTrack fit: momentum error (y-component) */
    float m_prFitMomentumZErr[20];                /** Fit result for the PRTrack fit: momentum error (z-component) */
    float m_prFitPositionXErr[20];                /** Fit result for the PRTrack fit: vertex error (x-component) */
    float m_prFitPositionYErr[20];                /** Fit result for the PRTrack fit: vertex error (y-component) */
    float m_prFitPositionZErr[20];                /** Fit result for the PRTrack fit: vertex error (z-component) */

    std::vector <int> m_prFitTracks;             /** PRTracks Ids for the given MCParticle */


  };
}

#endif /* TRACKINGOUTPUTMODULE_H_ */
