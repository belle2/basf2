/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <TCanvas.h>
#include <TH1.h>
#include <TLine.h>
#include <dqm/analysis/modules/DQMHistAnalysis.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisCDCDedxModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
    * constructor
    */
    DQMHistAnalysisCDCDedxModule();

    /**
    * destructor
    */
    virtual ~DQMHistAnalysisCDCDedxModule();

    /**
    * init function for default values
    */
    virtual void initialize() override;

    /**
    * begin each run
    */
    virtual void beginRun() override;

    /**
    * event by event function
    */
    virtual void event() override;

    /**
    * end of each run
    */
    virtual void endRun() override;

    /**
    * terminating at the end of last run
    */
    virtual void terminate() override;

    /**
    * funtion to compute mean and sigma of dEdx distribution
    */
    void computedEdxMeanSigma();

    /**
    * funtion to trending plots for mean and sigma of dEdx distribution
    */
    void computedEdxBandPlot();


  private:

    /** Save fitted mean and sigma for the 'dedx' values */

    TCanvas* c_CDCdedxMean = nullptr; /**< canvas for dedx mean value */
    TCanvas* c_CDCdedxSigma = nullptr; /**< canvas for dedx sigma value*/

    TH1F* h_CDCdedxMean = nullptr; /**< histogram for dedx mean value*/
    TH1F* h_CDCdedxSigma = nullptr; /**< histogram for dedx sigma value */

    TF1* f_fGaus = nullptr; /**< Gaus fit function for dEdx dist*/

    TLine* tLine = nullptr; /**< Ref line for dEdx fit mean */

    std::string runstatus; /**< Status of run quality */
    std::string runnumber; /**< Current Run number */

    double dedxmean; /**< fit value for dedx mean */
    double dedxsigma; /**< fit value for dedx sigma */


  };
} // end namespace Belle2

