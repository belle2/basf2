/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>

#include <cdc/dbobjects/CDCGeometry.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/geometry/CDCGeometryParConstants.h>

#include <cdc/calibration/CDCdEdx/CDCDedxWireGainAlgorithm.h>

#include <cdc/dbobjects/CDCDedxWireGain.h>
#include <cdc/dbobjects/CDCDedxCosineCor.h>
#include <cdc/dbobjects/CDCDedx1DCell.h>
#include <framework/database/Database.h>
#include <framework/database/DBStore.h>
#include <framework/database/Configuration.h>

#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <TTree.h>
#include <TMap.h>
#include <TPaveText.h>
#include <TF1.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TLegend.h>

/**
 * @brief Container for wire gain data
 */
struct WireGainData {
  /**
   * @brief Gain values for individual wires
   */
  std::vector<double> wiregain;

  /**
   * @brief Mean gain values per layer
   */
  std::vector<double> layermean;
};

/**
* @brief Container for cosine gain data
*/
struct CosGainData {
  /**
   * @brief Gain correction factors for cos(theta) bins
   */
  std::vector<double> cosgain;

  /**
   * @brief cos(theta) bin centers
   */
  std::vector<double> costh;
};

/**
* @brief Container for 1D gain data
*/
struct OnedData {
  /**
   * @brief Inner cell dE/dx values
   */
  std::vector<double> inner1D;

  /**
   * @brief Outer cell dE/dx values
   */
  std::vector<double> outer1D;

  /**
   * @brief Entrance angle values
   */
  std::vector<double> Enta;
};

namespace Belle2 {

  /**
   * A validation algorithm for CDC dE/dx electron
   *
   */
  class CDCDedxValidationAlgorithm : public CalibrationAlgorithm {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the algorithm.
     */
    CDCDedxValidationAlgorithm();

    /**
     * Destructor
     */
    virtual ~CDCDedxValidationAlgorithm() {}

    /**
    * @brief Draw dE/dx per run histogram canvas.
    * @param htemp Histogram map (run number -> TH1D pointer)
    * @param namesfx Suffix to distinguish output
    */
    void printCanvasRun(std::map<int, TH1D*>& htemp, std::string namesfx);

    /**
     * @brief Draw dE/dx histograms across bins.
     * @param htemp Vector of histograms
     * @param namesfx Suffix to distinguish output
     * @param svar Variable name for binning
     */
    void printCanvas(std::vector<TH1D*>& htemp, std::string namesfx, std::string svar);

    /**
     * @brief Perform Gaussian fit with range on a histogram.
     * @param temphist Histogram to fit
     * @param status Status string updated based on fit result
     */
    void fitGaussianWRange(TH1D*& temphist, std::string& status);

    /**
    * function to get extract calibration run/exp
    */
    void getExpRunInfo();

    /**
     * @brief Define dE/dx histograms for plotting.
     * @param htemp Vector to hold histograms
     * @param var Variable name
     * @param stype charge type
     */
    void defineHisto(std::vector<TH1D*>& htemp, std::string var, std::string stype);

    /**
     * @brief Validate wire gain data using dE/dx histograms
     * @param hdedxhit Vector of dE/dx hit histograms
     */
    void wireGain(std::vector<TH1D*>& hdedxhit);

    /**
     * @brief Plot dE/dx vs wire number.
     * @param temp Vector of histograms
     * @param namesfx Suffix to distinguish output
     * @param vdedx_mean Vector of mean dE/dx values
     */
    void printCanvasWire(std::vector<TH1D*> temp, std::string namesfx, const std::vector<double>& vdedx_mean);

    /**
     * @brief Perform full Gaussian fit and extract parameters.
     * @param hist Histogram to fit
     * @param mean Fitted mean
     * @param meanErr Error on the mean
     * @param sigma Fitted sigma
     * @param sigmaErr Error on the sigma
     */
    void fit(TH1D*& hist, double& mean, double& meanErr, double& sigma, double& sigmaErr);

    /**
    * @brief Validate dE/dx using bhabha sample (vs run, cosine)
    */
    void bhabhaValidation();

    /**
    * @brief Validate dE/dx using radee sample (vs momentum, injection time)
    */
    void radeeValidation();

    /**
     * @brief Set bin edges for injection time.
     * @param vtlocaledges Vector of time bin edges
     */
    void defineTimeBins(std::vector<double>& vtlocaledges);

    /**
     * @brief Draw dE/dx histograms for momentum and cosine bins.
     * @param htemp Array of vectors of histograms
     * @param namesfx Suffix to distinguish output
     * @param svar Variable name ("momentum" or "cosTheta")
     */
    void printCanvasdEdx(std::array<std::vector<TH1D*>, 2>& htemp, std::string namesfx, std::string svar);

    /**
     * @brief Get time bin label string
     * @param tedges Time edge
     * @param it Index of the bin
     * @return Formatted time bin label string
     */
    std::string getTimeBinLabel(const double& tedges, const int& it)
    {
      std::string label = "";
      if (tedges < 2e4)label = Form("%0.01f-%0.01fK", m_tedges[it] / 1e3, m_tedges[it + 1] / 1e3);
      else if (tedges < 1e5)label = Form("%0.0f-%0.0fK", m_tedges[it] / 1e3, m_tedges[it + 1] / 1e3);
      else label = Form("%0.01f-%0.01fM", m_tedges[it] / 1e6, m_tedges[it + 1] / 1e6);
      return label;
    }

    /**
     * @brief Set testing payload name
     * @param testingPayloadName Name of the payload
     */
    void setTestingPayload(const std::string& testingPayloadName)
    {
      m_testingPayloadName = testingPayloadName;
    }

    /**
     * @brief Set Global Tag name
     * @param globalTagName Name of the global tag
     */
    void setGlobalTag(const std::string& globalTagName)
    {
      m_GlobalTagName = globalTagName;
    }

    /**
     * @brief Set text cosmetics for TPaveText
     * @param pt TPaveText object
     * @param color Color to apply
     */
    void setTextCosmetics(TPaveText pt, Color_t color)
    {
      pt.SetBorderSize(0);
      pt.SetShadowColor(kWhite);
      pt.SetTextColor(color);
    }

    /**
    * @brief Plot summary statistics of selected events
    */
    void plotEventStats();

    /**
     * @brief Load database payload for given run.
     * @param experiment Experiment number
     * @param run Run number
     */
    void DatabaseIN(int experiment, int run);

    /**
     * @brief Retrieve wire gain data from DB
     * @param experiment Experiment number
     * @param run Run number
     * @return WireGainData structure
     */
    WireGainData getwiregain(int experiment, int run);

    /**
     * @brief Retrieve cosine gain data from DB
     * @param experiment Experiment number
     * @param run Run number
     * @return CosGainData structure
     */
    CosGainData getcosgain(int experiment, int run);

    /**
     * @brief Retrieve 1D gain data from DB.
     * @param experiment Experiment number
     * @param run Run number
     * @return OnedData structure
     */
    OnedData getonedgain(int experiment, int run);

    /**
    * @brief Clear current DB pointers and state
    */
    void resetDatabase();

  protected:

    /**
    * @brief Main calibration method
    */
    virtual EResult calibrate() override;

  private:
    double m_sigmaR;  /**< fit dedx dist in sigma range */

    int m_dedxBins;  /**< bins for dedx histogram */
    double m_dedxMin; /**< min range of dedx */
    double m_dedxMax; /**< max range of dedx */

    int m_cosBins; /**< bins for cosine */
    double m_cosMin; /**< min range of cosine */
    double m_cosMax; /**< max range of cosine */

    int m_momBins; /**< bins for momentum */
    double m_momMin; /**< min range of momentum */
    double m_momMax; /**< max range of momentum */

    double* m_tedges; /**< internal time array (copy of vtlocaledges) */
    unsigned int m_tbins;  /**< internal time bins */

    std::array<std::string, 2> m_sring{"ler", "her"}; /**< injection ring name */

    std::string m_suffix; /**< suffix string to separate plots */

    DBObjPtr<CDCGeometry> m_cdcGeo; /**< Geometry of CDC */
    DBObjPtr<CDCDedxWireGain> m_DBWireGains; /**< Wire gain DB object */

    /** Testing payload location. */
    std::string m_testingPayloadName = "";

    /** Global Tag name. */
    std::string m_GlobalTagName = "";

    /** Event metadata. */
    StoreObjPtr<EventMetaData> m_EventMetaData;

  };
} // namespace Belle2
