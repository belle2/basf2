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
#include "TF1.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TString.h"
#include "TRandom.h"
#include "TMath.h"
#include "TFitter.h"
#include "TGraphErrors.h"
#include "TCanvas.h"
#include <reconstruction/dbobjects/CDCDedxHadronCor.h>
#include <framework/database/DBObjPtr.h>

namespace Belle2 {

  class HadronSaturation {


  public:

    HadronSaturation();
    HadronSaturation(double alpha, double gamma, double delta, double power, double ratio);
    virtual ~HadronSaturation() { clear(); };

    // set the parameters
    void setParameters(double par[]);

    // set the parameters
    void setParameters();

    // fill the vectors below
    void fillSample(TString infilename);

    // print a sample of events
    void printEvents(int firstevent, int nevents);

    // perform the hadron saturation fit
    void fitSaturation();

    // clear the vectors
    void clear();

    // set the number of cosine bins
    void setCosBins(int nbins) { m_cosbins = nbins; }

    // set the hadron saturation function flag
    void setFlag(int flag) { m_flag = flag; }

    // some helper functions for the hadron saturation correction
    double myFunction(double alpha, double gamma, double delta, double power, double ratio);
    static void minuitFunction(int& nDim, double* gout, double& result, double* para, int flg);

    double D2I(double cosTheta, double D, double alpha, double gamma, double delta, double power, double ratio) const;
    double I2D(double cosTheta, double I, double alpha, double gamma, double delta, double power, double ratio) const;

  private:

    // flag for saturation function
    int m_flag;

    // the number of cosine bins
    int m_cosbins;

    // the parameters for the hadron saturation correction
    double m_alpha;
    double m_gamma;
    double m_delta;
    double m_power;
    double m_ratio;

    std::vector< double > m_dedx;      // a vector to hold dE/dx measurements
    std::vector< double > m_dedxerror; // a vector to hold dE/dx errors
    std::vector< double > m_betagamma; // a vector to hold beta-gamma values
    std::vector< double > m_costheta;  // a vector to hold cos(theta) values
    DBObjPtr<CDCDedxHadronCor> m_DBHadronCor;
  };
}
