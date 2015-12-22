/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Roberto Stroili                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPLASERCALIBRATORMODULE_H
#define TOPLASERCALIBRATORMODULE_H

#include <framework/core/Module.h>
#include <string>
// #include <TH1F.h>

class TH1F;
class TF1;
class TTree;

namespace Belle2 {

  /**
   * 3 gaussian fitting function
   * (under development)
   */
  double threegauss(double* x, double* par);

  /**
   * Laser calibration module
   * (under development)
   */
  class TOPLaserCalibratorModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPLaserCalibratorModule();

    /**
     * Destructor
     */
    virtual ~TOPLaserCalibratorModule();

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
     * Do fits , clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

  private:

    /**
     * number of channels per module, storage windows per channel
     */
    enum {c_NumChannels = 512,
          c_maxLaserFibers = 9
         };

    std::string m_histogramFileName; /**< output file name for histograms */
    std::string m_referenceFileName; /**< input file name for fitting parameters */
    int m_barID;                     /**< ID of TOP module to calibrate */
    int m_runLow;          /**< IOV: from run */
    int m_runHigh;         /**< IOV: to run */
    bool m_gaussFit;       /**< fitting function flag: if true fit single gaussian */

    TH1F* m_histo[c_NumChannels]; /**< profile histograms */
    TTree* m_fittingParmTree; /**< tree with fitting parameters */
    /**
     * information stored in the root tree
     * required to fit the 3-gaussian function.
     */
    int m_pix;   /**< pixel number */
    int m_nfibers; /**< number of fibers illuminating the pixel */
    int m_fibers[c_maxLaserFibers]; /**< fibers illuminating the pixel */
    float m_phnorm[c_maxLaserFibers]; /**< (photon) normalization factor for the fiber illuminating the pixel */
    float m_phmean[c_maxLaserFibers]; /**< (photon) time for the fiber illuminating the pixel */
    float m_phstd[c_maxLaserFibers]; /**< (photon) resolution for the fiber illuminating the pixel */
    float m_diginorm[c_maxLaserFibers]; /**< (digi)  normalization factor for the fiber illuminating the pixel */
    float m_digimean[c_maxLaserFibers]; /**< (digi) time for the fiber illuminating the pixel */
    float m_digistd[c_maxLaserFibers]; /**< (digi) resolution for the fiber illuminating the pixel */

    /**
     * make fit with the 3-gaussian function.
     */
    TF1* makeFit(TH1F* h, int ch);
    /**
     * make fit with a single gaussian function.
     */
    TF1* makeGFit(TH1F* h);

  };

} // Belle2 namespace

#endif
