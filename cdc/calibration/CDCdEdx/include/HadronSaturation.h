/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <fstream>

#include "TFile.h"
#include "TTree.h"
#include "TRandom.h"
#include "TMath.h"
#include "TFitter.h"

#include <framework/database/DBObjPtr.h>
#include <cdc/utilities/CDCDedxHadSat.h>

namespace Belle2 {

  /**
  * Class to perform the hadron saturation calibration
  */
  class HadronSaturation {

  public:

    /**
    * Constructor: Sets the description, the properties and the parameters of the algorithm.
    */
    HadronSaturation();

    /**
    * set the input variables
    */
    HadronSaturation(double alpha, double gamma, double delta, double power, double ratio, int cosbins);

    /**
    * Destructor
    */
    virtual ~HadronSaturation() { clear(); };

    /**
    * fill the vectors below
    */
    void fillSample(TString infilename);

    /**
    * print a sample of events
    */
    void printEvents(int firstevent, int nevents);

    /**
    * perform the hadron saturation fit
    */
    void fitSaturation();

    /**
    * clear the vectors
    */
    void clear();

    /**
    * set the number of cosine bins
    */
    void setCosBins(int nbins) { m_cosbins = nbins; }

    /**
    * some helper functions for the hadron saturation correction
    */
    double myFunction(double alpha, double gamma, double delta, double power, double ratio);

    /**
    * functions for the hadron saturation correction
    */
    static void minuitFunction(int& nDim, double* gout, double& result, double* para, int flg);

  private:

    int m_cosbins;  /**< the number of cosine bins */

    double m_alpha; /**< the alpha parameter for the hadron saturation correction */
    double m_gamma; /**< the gamma parameter for the hadron saturation correction */
    double m_delta; /**< the delta parameter for the hadron saturation correction */
    double m_power; /**< the power parameter for the hadron saturation correction */
    double m_ratio; /**< the ratio parameter for the hadron saturation correction */

    std::vector< double > m_dedx;      /**< a vector to hold dE/dx measurements */
    std::vector< double > m_dedxerror; /**< a vector to hold dE/dx errors */
    std::vector< double > m_betagamma; /**< a vector to hold beta-gamma values */
    std::vector< double > m_costheta;  /**< a vector to hold cos(theta) values */

  };
}
