/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPBETASCANMODULE_H
#define TOPBETASCANMODULE_H

#include <framework/core/Module.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction/TOPreco.h>
#include <top/reconstruction/TOPtrack.h>
#include <framework/gearbox/Const.h>

#include <string>
#include "TFile.h"
#include "TH1.h"

namespace Belle2 {

  /**
   * Module to reconstructs particle beta using extended maximum likelihood
   */
  class TOPbetaScanModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPbetaScanModule();

    /**
     * Destructor
     */
    virtual ~TOPbetaScanModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

    /**
     * Prints module parameters.
     */
    void printModuleParams() const;

  private:
    /**
     * Return extrapolated tracks
     * @param tracks vector of TOPtracks to return
     * @param chargedStable particle hypothesis used for extrapolation
     */
    void getTracks(std::vector<TOP::TOPtrack>& tracks, Const::ChargedStable chargedStable);

    /**
     * Reconstruct particle velocity beta
     * @param reco reconstruction object
     * @param track TOPtrack
     * @return beta
     */
    double reconstructBeta(TOP::TOPreco& reco, TOP::TOPtrack& track);

    /**
     * Return log likelihood for given beta
     * @param reco reconstruction object
     * @param track TOPtrack
     * @param beta particle velocity beta
     * @return log likelihood
     */
    double getLogLikelihood(TOP::TOPreco& reco, TOP::TOPtrack& track, double beta);

    /**
     * Improve precision of log likelihood maximum position using bisection
     * @param reco reconstruction object
     * @param track TOPtrack
     * @param Beta array of particle velocities beta
     * @param LogL array of log likelihoods
     */
    void improvePrecision(TOP::TOPreco& reco, TOP::TOPtrack& track,
                          double Beta[], double LogL[]);

    /**
     * convert a number to printable string
     * @param number number
     * @return printable string
     */
    std::string numberToString(int number);

    std::string m_outputFileName;  /**< output file name */
    double m_betaMin;  /**< lower limit of beta range to scan */
    double m_betaMax;  /**< upper limit of beta range to scan */
    int m_numPoints;   /**< number of scan points */
    int m_numBisect;   /**< number of bisection steps */
    int m_numBins;     /**< number of histogram bins to histogram the results */
    int m_numScanHistograms; /**< number of scan histograms to be written to file */
    double m_minBkgPerBar;    /**< minimal assumed background photons per bar */
    double m_scaleN0;         /**< scale factor for N0 */
    double m_electronicJitter;     /**< r.m.s of electronic jitter */
    double m_maxTime;      /**< optional time limit for photons */
    int m_everyNth;     /**< randomly choose every Nth event */

    TFile* m_file;           /**< TFile */
    TH1F* m_betaHistogram;   /**< 1D histogram for the results */
    std::vector<TH1F*> m_scanHistograms; /**< scan histograms for control */

    TOP::TOPGeometryPar* m_topgp;   /**< geometry parameters */

  };

} // Belle2 namespace

#endif
