/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Wenlong Yuan                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef LASERCALIBRATORFIT_H
#define LASERCALIBRATORFIT_H

#include <TH1F.h>
#include <TF1.h>
#include <string>

namespace Belle2 {

  /**
   * single Crystal Ball fuction
   * (under development)
   */
  double fcnCB(double* x, double* par);

  /**
   * double Crystal Ball fuction
   * (under development)
   */
  double fcnCB2(double* x, double* par);

  /**
   * A class do laser calibration fit
   * provide different fitting method
   * (under development)
   */

  class LaserCalibratorFit {
  public:
    /**
     * Constructor
     */
    LaserCalibratorFit(int moduleID);

    /**
     * Destructor
     */
    ~LaserCalibratorFit();


    void setHist(TH1F* hist[512]); /**< set time hist of 512 pixels in one moduleID */
    void setFitMethod(std::string method = "gauss") {m_fitMethod = method;} /**< gauss: single gaussian; cb: single Crystal Ball; cb2: double Crystal Ball */
    void setFitRange(double xmin = -200, double xmax = 200) {m_xmin = xmin; m_xmax = xmax;} /**< fit range [xmin, xmax] */
    int  fitPixel(int pixelID); /**< fit a specific pixelID (0 - 511) */
    int  fitAllPixels(); /**< fit all pixels in on slot */
    void writeFile(std::string outfile); /**< write fitting results to a root file */

    int getModuleID() {return m_moduleID;} /**< get moduleID in the fit */
    double getPixelChisq(int pixelID); /**< get fit chi^2 for a pixel */
    std::string getFitMethod() {return m_fitMethod;} /**< get fitting method in the fit */

  private:
    TH1F* m_hist[512]; /**< time hist of 512 pixels */
    int m_moduleID; /**< one moduleID/slot */
    TF1* m_func[512]; /**< fitting function */
    std::string m_fitMethod; /**< fitting method */

    double m_xmin; /**< fitting low-edge */
    double m_xmax; /**< fitting upper-edge */

    /**
     * Fit process using single gaussian function
     */
    TF1* makeGFit(TH1F* h);

    /**
     * Fit process using single Crystal Ball fuction
     */
    TF1* makeCBFit(TH1F* h);

    /**
     * Fit process using double Crystal Ball fuction
     * @param pixelID  refers to a typical time separation of two main peaks
     * @param minOut  minimum printing
     * need more studies
     */
    TF1* makeCB2Fit(TH1F* h, int pixelID, bool minOut);
  };

}//Belle2 namespace

#endif
