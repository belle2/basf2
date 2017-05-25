#include <TFile.h>
#include <TObject.h>

#pragma once


namespace Belle2 {

  class NoKickCuts: public TObject {

    //  This class is an auxiliary class that implement methods to access
    //  to a single cut, used in NoKickRTSel class.


  public:
    NoKickCuts() {}

    NoKickCuts(std::string m_fileName)
    {
      if (m_fileName.size() != 0) FillCuts(m_fileName);
    }

    std::vector<std::vector <std::vector<std::vector <double>>>> m_cutNorm;
    std::vector<std::vector <std::vector<std::vector <double>>>> m_cutPow;
    std::vector<std::vector <std::vector<std::vector <double>>>> m_cutBkg;

    int m_nbinpar = 5;
    int m_nbinlay = 7;

    enum Eminmax {
      min,
      max
    };

    enum Eparameter {
      omega,
      d0,
      phi0,
      z0,
      tanlambda
    };

    enum Ecutname {
      Norm,
      Pow,
      Bkg
    };

    // This methods selects 2 cuts (minimum and maximum inside a vector) from
    // the information of theta, momentum, the layers and the parameter. The
    // method takes the cuts from cutfile using the method getCuts and then
    // evaluate the effective cut using theta and momentum behaviour, from the
    // function  Norm/(p^Pow*sqrt(sin(theta)))+Bkg.
    std::vector<double> cutSelector(double sintheta, double momentum, int layer1, int layer2, Eparameter par);

    // This methods select the cuts as function parameters from TH3F cointained
    // in cutfile in function of track parameter and layer only. Eminmax represent
    // if you want the maximum or minimum cut, instead Ecutname are the three
    // parameters of the theta-p parametrization of the cuts (see the enum type)
    double getCut(int layer1, int layer2, Eparameter par, Eminmax m, Ecutname cut);

    //  This method fill the cuts (used in NoKickCutsEval method) to create the
    //  cutfile.
    void FillCuts(std::string m_fileName);

    ClassDef(NoKickCuts, 1);
  };

} //end namespace Belle2
