/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCTRACKCANDCLASSIFIER_H_
#define MCTRACKCANDCLASSIFIER_H_

#include <framework/core/Module.h>
#include <string>
#include <TTree.h>
#include <TFile.h>
#include <TList.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>
#include <time.h>

#include <vxd/dataobjects/VXDTrueHit.h>
#include <mdst/dataobjects/MCParticle.h>
#include <tracking/dataobjects/MCParticleInfo.h>
#include <framework/datastore/SelectSubset.h>
#include <genfit/TrackCand.h>

namespace Belle2 {

  class  MCParticleInfo;

  /** The MCTrackCandClassifier Definition Module
   *
   * this module classifies the MCTrackCands in
   * - idealMCTrackCands: MCTrackCand
   * - fineMCTrackCands (not produced yet)
   * - nastyMCTrackCands (not produced yet)
   * on the basis of their properties in order to study the inefficiencies of
   * the pattern recognition and the fitter
   */

  class MCTrackCandClassifierModule : public Module {

  public:

    /**
     * Constructor of the module.
     */
    MCTrackCandClassifierModule();

    /**
     * Destructor of the module.
     */
    ~MCTrackCandClassifierModule() {};

    /**
     *Initializes the Module.
     */
    void initialize() override;

    void beginRun() override;

    void event() override;

    void endRun() override;

    /**
     * Termination action.
     */
    void terminate() override;

  protected:

    //    SelectSubset< genfit::TrackCand > m_selector; /**<selector*/

    std::string m_mcTrackCandsColName; /**< TrackCand list name*/
    std::string m_mcParticlesName;  /**< MCParticle list name*/
    std::string m_rootFileName;   /**< root file name */
    int m_nSigma;   /**< nSigma dR */
    bool m_usePXD;
    bool m_applyAnnulus;
    bool m_applySemiplane;
    bool m_applyLap;
    bool m_applyWedge;
    bool m_removeBadHits;
    int m_minHit;
    double m_fraction;

    int nWedge;
    int nBarrel;

    TFile* m_rootFilePtr; /**< pointer at root file used for storing histograms */

    TList* m_histoList;
    TH3F* m_h3_MCParticle;
    TH3F* m_h3_idealMCTrackCand;
    TH3F* m_h3_MCTrackCand;

    TH1F* m_h1_thetaMS_SVD;
    TH1F* m_h1_thetaMS_PXD;
    TH1F* m_h1_dR;
    TH1F* m_h1_dRoverR;
    TH1F* m_h1_distOVERdR;
    TH1F* m_h1_hitRadius_accepted;
    TH1F* m_h1_hitRadius_rejected;
    TH1F* m_h1_hitDistance_accepted;
    TH1F* m_h1_hitDistance_rejected;

    TH1F* m_h1_MCTrackCandNhits;
    TH1F* m_h1_firstRejectedHit;
    TH1F* m_h1_firstRejectedOVERMCHit;

    TH1F* m_h1_lapTime;
    TH1F* m_h1_timeDifference;
    TH1F* m_h1_diffOVERlap;

    TH1F* m_h1_nGoodTrueHits;
    TH1F* m_h1_nBadTrueHits;
    TH1F* m_h1_nGood1dInfo;
    TH1F* m_h1_nBad1dInfo;

  private:

    double  getXintersect(double d0, double omega, double R);
    double semiPlane(TVector3 vertex, TVector3 center, TVector3 hit);
    bool isInSemiPlane(double semiPlane, double omega);
    double theDistance(TVector3 center, TVector3 hit);
    bool isInAnnulus(double hitDistance, double R, double dR);
    bool isFirstLap(double FirstHitTime, double HitTime, double LapTime);

    TH3F* createHistogram3D(const char* name, const char* title,
                            Int_t nbinsX, Double_t minX, Double_t maxX, const char* titleX,
                            Int_t nbinsY, Double_t minY, Double_t maxY, const char* titleY,
                            Int_t nbinsZ, Double_t minZ, Double_t maxZ, const char* titleZ,
                            TList* histoList = NULL);

    TH3F* createHistogram3D(const char* name, const char* title,
                            Int_t nbinsX, Double_t* binsX, const char* titleX,
                            Int_t nbinsY, Double_t* binsY, const char* titleY,
                            Int_t nbinsZ, Double_t* binsZ, const char* titleZ,
                            TList* histoList = NULL);

    TH1* duplicateHistogram(const char* newname, const char* newtitle,
                            TH1* h, TList* histoList = NULL);

    TH1F* createHistogramsRatio(const char* name, const char* title,
                                TH1* hNum, TH1* hDen, bool isEffPlot,
                                int axisRef);

    void addEfficiencyPlots(TList* graphList = NULL);

    void addInefficiencyPlots(TList* graphList = NULL);

    float compute_dR(double thetaMS, double omega);
    float compute_thetaMS(MCParticleInfo& mcParticleInfo, VXDTrueHit* aTrueHit);
  };
}

#endif /* MCTrackCandClassifierModule_H_ */
