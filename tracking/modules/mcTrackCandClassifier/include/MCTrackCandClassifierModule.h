/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>
#include <TFile.h>
#include <TList.h>
#include <TH1F.h>
#include <TH3F.h>
#include <time.h>

#include <tracking/dataobjects/MCParticleInfo.h>
#include <vxd/dataobjects/VXDTrueHit.h>

namespace Belle2 {

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

    /// Constructor of the module.
    MCTrackCandClassifierModule();

    /// Initializes the Module.
    void initialize() override;

    /// Begin run.
    void beginRun() override;

    /// Event function.
    void event() override;

    /// End run.
    void endRun() override;

    /// Termination action.
    void terminate() override;

  protected:
    /// TrackCand list name
    std::string m_mcTrackCandsColName;
    /// MCParticle list name
    std::string m_mcParticlesName;
    /// root file name
    std::string m_rootFileName;
    /// nSigma dR
    int m_nSigma;
    /// Whether to use PXD
    bool m_usePXD;
    /// Whether to require that the hit is in the expected annulus.
    bool m_applyAnnulus;
    /// Wether to require that the hit is in the expected semiplane.
    bool m_applySemiplane;
    /// Whether to require that the hit belongs to the first lap in the transverse plane.
    bool m_applyLap;
    /// Whether to require that the hit belong to the barrel part of the SVD.
    bool m_applyWedge;
    /// Whether to remove the clusters that do not satisfy the criteria from the idealMCTrackCands.
    bool m_removeBadHits;
    /// Minimum number of 1D Clusters to classify the MCTrackCand as ideal.
    int m_minHit;
    /// Fraction of lap
    double m_fraction;

    /// Counter for hits on wedged sensors.
    int nWedge = 0;
    /// Counter for hits on barrel sensors.
    int nBarrel = 0;

    /// Pointer to root file used for storing histograms
    TFile* m_rootFilePtr = nullptr;

    TList* m_histoList = nullptr; /**< List of histograms */
    TH3F* m_h3_MCParticle = nullptr; /**< Histogram */
    TH3F* m_h3_idealMCTrackCand = nullptr; /**< Histogram */
    TH3F* m_h3_MCTrackCand = nullptr; /**< Histogram */

    TH1F* m_h1_thetaMS_SVD = nullptr; /**< Histogram */
    TH1F* m_h1_thetaMS_PXD = nullptr; /**< Histogram */
    TH1F* m_h1_dR = nullptr; /**< Histogram */
    TH1F* m_h1_dRoverR = nullptr; /**< Histogram */
    TH1F* m_h1_distOVERdR = nullptr; /**< Histogram */
    TH1F* m_h1_hitRadius_accepted = nullptr; /**< Histogram */
    TH1F* m_h1_hitRadius_rejected = nullptr; /**< Histogram */
    TH1F* m_h1_hitDistance_accepted = nullptr; /**< Histogram */
    TH1F* m_h1_hitDistance_rejected = nullptr; /**< Histogram */

    TH1F* m_h1_MCTrackCandNhits = nullptr; /**< Histogram */
    TH1F* m_h1_firstRejectedHit = nullptr; /**< Histogram */
    TH1F* m_h1_firstRejectedOVERMCHit = nullptr; /**< Histogram */

    TH1F* m_h1_lapTime = nullptr; /**< Histogram */
    TH1F* m_h1_timeDifference = nullptr; /**< Histogram */
    TH1F* m_h1_diffOVERlap = nullptr; /**< Histogram */

    TH1F* m_h1_nGoodTrueHits = nullptr; /**< Histogram */
    TH1F* m_h1_nBadTrueHits = nullptr; /**< Histogram */
    TH1F* m_h1_nGood1dInfo = nullptr; /**< Histogram */
    TH1F* m_h1_nBad1dInfo = nullptr; /**< Histogram */

  private:

    /** Function to get semiplane.
     * @param vertex: Vertex position
     * @param center: center positon
     * @param hit: hit position
     * @return difference in y-slope between hit and semiplane
     */
    double semiPlane(TVector3 vertex, TVector3 center, TVector3 hit);

    /** Function to check if a omega value is in a given semiPlane.
     * @param semiPlane: slope of semiplane
     * @param omega: Signed curvature
     * @return true if in semiplane
     */
    bool isInSemiPlane(double semiPlane, double omega);

    /** Get distance between two points.
     * @param center: center position
     * @param hit: hit position
     * @return Distance between the two hits
     */
    double theDistance(TVector3 center, TVector3 hit);

    /** Function to check if hitDistance is within a given annulus.
     * @param hitDistance: distance between two hits
     * @param R: radius for annulus
     * @param dR: width of annulus
     * @return true if in annulus
     */
    bool isInAnnulus(double hitDistance, double R, double dR);

    /** Function to check if a hitTime is within a given lapTime,
     * under consideration of m_fraction and with respect to first hit.
     * @param FirstHitTime: time of first hit
     * @param HitTime: hit time
     * @param LapTime: lap time
     * @return true if in first lap
     */
    bool isFirstLap(double FirstHitTime, double HitTime, double LapTime);

    /** Create a 3D ROOT Histogram.
     * @param name: name of histogram
     * @param title: title of histogram
     * @param nbinsX: number of bins on x axis
     * @param minX: minimal x value
     * @param maxX: maximal x value
     * @param titleX: x axis title
     * @param nbinsY: number of bins on y axis
     * @param minY: minimal y value
     * @param maxY: maximal y value
     * @param titleY: y axis title
     * @param nbinsZ: number of bins on z axis
     * @param minZ: minimal z value
     * @param maxZ: maximal z value
     * @param titleZ: z axis title
     * @param histoList: list of histograms to add this histogram to
     * @return the histogram
     */
    TH3F* createHistogram3D(const char* name, const char* title,
                            Int_t nbinsX, Double_t minX, Double_t maxX, const char* titleX,
                            Int_t nbinsY, Double_t minY, Double_t maxY, const char* titleY,
                            Int_t nbinsZ, Double_t minZ, Double_t maxZ, const char* titleZ,
                            TList* histoList = nullptr);

    /** Create a 3D ROOT Histogram.
     * @param name: name of histogram
     * @param title: title of histogram
     * @param nbinsX: number of bins on x axis
     * @param binsX: x bins
     * @param titleX: x axis title
     * @param nbinsY: number of bins on y axis
     * @param binsY: y bins
     * @param titleY: y axis title
     * @param nbinsZ: number of bins on z axis
     * @param binsZ: z bins
     * @param titleZ: z axis title
     * @param histoList: list of histograms to add this histogram to
     * @return the histogram
     */
    TH3F* createHistogram3D(const char* name, const char* title,
                            Int_t nbinsX, Double_t* binsX, const char* titleX,
                            Int_t nbinsY, Double_t* binsY, const char* titleY,
                            Int_t nbinsZ, Double_t* binsZ, const char* titleZ,
                            TList* histoList = nullptr);

    /** Function to clone a histogram.
     * @param newname: new histogram name
     * @param newtitle: new title
     * @param h: histogram to clone
     * @param histoList: list of histograms to add this histogram to
     * @return the duplicated histogram
     */
    TH1* duplicateHistogram(const char* newname, const char* newtitle,
                            TH1* h, TList* histoList = nullptr);

    /** Function to create a ratio histogram from two histograms.
     * @param name: name of histogram
     * @param title: title of histogram
     * @param hNum: histogram for the numerator of the ratio
     * @param hDen: histogram for the denominator of the ratio
     * @param isEffPlot: if the result is a efficiency plot
     * @param axisRef: reference axis to use. (x=0, y=1, z=2)
     * @return the histogram
     */
    TH1F* createHistogramsRatio(const char* name, const char* title,
                                TH1* hNum, TH1* hDen, bool isEffPlot,
                                int axisRef);

    /** Function to create efficiency plots and add them to list.
     * @param graphList: pointer to list to add the plots to
     */
    void addEfficiencyPlots(TList* graphList = nullptr);

    /** Function to create inefficiency plots and add them to list.
     * @param graphList: pointer to list to add the plots to
     */
    void addInefficiencyPlots(TList* graphList = nullptr);

    /** Calculate dR.
     * @param thetaMS: thetaMS of a hit
     * @param omega: omega value
     * @return dR value
     */
    float compute_dR(double thetaMS, double omega);

    /** Calculate thetaMS.
     *
     * @param mcParticleInfo: mcParticleInfo object
     * @param aTrueHit: pointer to vxd ture hit
     * @return thetaMS value
     */
    float compute_thetaMS(MCParticleInfo& mcParticleInfo, VXDTrueHit* aTrueHit);
  };
}
