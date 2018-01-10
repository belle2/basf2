/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Wenlong Yuan                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TH1F.h>
#include <TF1.h>
#include <string>

namespace Belle2 {
  namespace TOP {

    /**
     * single Crystal Ball fuction
     * (under development)
     * @param x  random variable in Crystal Ball pdf
     * @param par  parameters in Crystal Ball pdf
     */
    double fcnCB(double* x, double* par);

    /**
     * double Crystal Ball fuction
     * (under development)
     * @param x  random variable in Crystal Ball pdf
     * @param par  parameters in Crystal Ball pdf
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
      explicit LaserCalibratorFit(unsigned moduleID);

      /**
       * Destructor
       */
      ~LaserCalibratorFit();

      /**
       * set time hist of all channels in one moduleID
       * @param hist  time hist vector
       */
      void setHist(const std::vector<TH1F*>& hist);

      /**
      * set time fit function
      * @param method  gauss: single gaussian; cb: single Crystal Ball; cb2: double Crystal Ball
      */
      void setFitMethod(std::string& method) {m_fitMethod = method;}

      /**
      * set x range in the fit
      * @param xmin  lower x
      * @param xmax  upper x
      */
      void setFitRange(double xmin = -200, double xmax = 200) {m_xmin = xmin; m_xmax = xmax;}

      /**
      * fit for a specific channel
      * @param channel  a specific channel (0 - 511)
      */
      int  fitChannel(unsigned channel);

      /**
      * write fit result to a root file
      * @param outfile  output file name
      */
      void writeFile(const std::string& outfile);

      /**
       * get the module ID in the fit
       */
      int getModuleID() const {return m_moduleID;}

      /**
       * get chi^2 in the fit
       * @param channel  chi^2 for a channel
       */
      double getFitChisq(unsigned channel);

      /**
       * get the fit method used in recent module
       */
      std::string getFitMethod() const {return m_fitMethod;}

      /**
       * get the center positon of hist max bin
       */
      std::vector<double> getMaxPos()  {return m_maxpos;}

      /**
       * get mean positon after fit
       */
      double getFitT()  {return m_fitT;}

      /**
       * returns the error mean positon after fit
       */
      double getFitTErr() {return m_fitTErr;}


    private:

      /**
       * Fit process using single gaussian function
       * @param h  hist in the fit
       */
      TF1* makeGFit(unsigned channel);

      /**
       * Fit process using single Crystal Ball fuction
       * @param h  hist in the fit
       */
      TF1* makeCBFit(unsigned channel);

      /**
       * Fit process using double Crystal Ball fuction
       * @param channel  refers to a typical time separation of two main peaks
       * @param minOut  minimum printing
       * need more studies
       */
      TF1* makeCB2Fit(unsigned channel, bool minOut);

      unsigned m_moduleID = 0; /**< one moduleID/slot */
      std::vector<double> m_maxpos; /**< center positon of hist max bin */
      double m_xmin = 0; /**< fitting low-edge */
      double m_xmax = 0; /**< fitting upper-edge */
      double m_fitT = 0; /**< mean position after fit */
      double m_fitTErr = 0.; /**< error on the mean position estimated by the fit */
      std::vector<TH1F*> m_hist; /**< time hist of 512 channels */
      std::vector<TF1*> m_func; /**< fitting function */
      std::string m_fitMethod; /**< fitting method */

    };
  }// TOP namespace
}//Belle2 namespace
