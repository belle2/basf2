/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Valerio Bertacchi                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <TFile.h>
#include <TObject.h>

#pragma once


namespace Belle2 {

  class NoKickCuts: public TObject {

    //  This class is an auxiliary class that implement methods to access
    //  to a single cut, used in NoKickRTSel class.


  public:
    //constructor of the class without arguments, used in cuts evaluations
    NoKickCuts() {}

    //constructor with name of the CutFile, used in cuts application
    NoKickCuts(std::string m_fileName)
    {
      if (m_fileName.size() != 0) FillCuts(m_fileName);
    }

    std::vector<std::vector <std::vector<std::vector <double>>>> m_cutNorm; //matrix of fit-parameter of cut, norm
    std::vector<std::vector <std::vector<std::vector <double>>>> m_cutPow; //matrix of fit-parameter of cut, power
    std::vector<std::vector <std::vector<std::vector <double>>>> m_cutBkg; //matrix of fit-parameter of cut, constant

    int m_nbinpar = 5; //number of track parameter
    int m_nbinlay = 7; //number of layers (IP too)

    enum Eminmax { //enum for minimum/maximum value of cuts
      min,
      max
    };

    enum Eparameter { //enum for parameters name
      omega,
      d0,
      phi0,
      z0,
      tanlambda
    };

    enum Ecutname { //enum for the cuts name
      Norm,
      Pow,
      Bkg
    };

    // This methods selects 2 cuts (minimum and maximum inside a vector) from
    // the information of theta, momentum, the layers and the parameter. The
    // method takes the cuts from cutfile using the method getCuts and then
    // evaluate the effective cut using theta and momentum behaviour, from the
    // function  Norm/(p^Pow*sqrt(sin(theta)))+Bkg.
    // input: (sin(angle), momentum, first layer, second layer, track parameter)
    // output: evaluated cut value
    std::vector<double> cutSelector(double sintheta, double momentum, int layer1, int layer2, Eparameter par);

    // This methods select the cuts as function parameters from TH3F cointained
    // in cutfile in function of track parameter and layer only. Eminmax represent
    // if you want the maximum or minimum cut, instead Ecutname are the three
    // parameters of the theta-p parametrization of the cuts (see the enum type)
    //input: (first layer, second layer, track parameter, min/max value, fit parameter)
    //output: (value of the fit parameter of a specific cut)
    double getCut(int layer1, int layer2, Eparameter par, Eminmax m, Ecutname cut);

    //  This method fill the cuts (used in NoKickCutsEval method) to create the
    //  cutfile.
    //input: (name of the cutFile)
    void FillCuts(std::string m_fileName);

    ClassDef(NoKickCuts, 1);
  };

} //end namespace Belle2
