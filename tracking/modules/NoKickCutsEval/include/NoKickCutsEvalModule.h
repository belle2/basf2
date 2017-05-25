#ifndef NOKICKCUTSEVAL_H
#define NOKICKCUTSEVAL_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <tracking/dataobjects/hitXP.h>
#include <tracking/modules/hitXP/hitXPModule.h>
#include <tracking/modules/NoKickCutsEval/NoKickCutsEvalModule.h>
#include <tracking/trackFindingVXD/sectorMapTools/NoKickCuts.h>
#include <tracking/trackFindingVXD/sectorMapTools/NoKickRTSel.h>
#include <TFile.h>
#include <math.h>


namespace Belle2 {

  //  This module evaluate the cuts used to select the training sample
  //  of the SectorMap. From simulated data files, during the events create
  //  for each track a vector of hitXP (see hitXP class) with the
  //  first hit on each layer of VXD (see NoKickRTSel class for detalis). Than
  //  fills some histogram with the distribution of difference of parameters
  //  betweeen following layers.
  //  During endrun evaluate the cuts on tails of these distibution using a
  //  funcion that define the percentage that has to be selected. Than these
  //  cuts are fitted in momentum and angular theta distiribution. In the end
  //  the parameters of the fits are put in a matrix (TH3F) on a TFile and
  //  they contains all the informations of the cuts. The output file
  // NoKickCuts.root has to be used duing training of SectorMap.


  class NoKickCutsEvalModule : public Module {
  public:
    NoKickCutsEvalModule();

    virtual ~NoKickCutsEvalModule();


    virtual void initialize() override;

    virtual void beginRun() override;

    virtual void event() override;

    virtual void endRun() override;

    virtual void terminate() override;

    enum Eparameters {
      omega,
      d0,
      phi0,
      z0,
      tanlambda
    };

    //  this method evaluate the difference of a required track parameter "par"
    //  between 2 hit. If is0 is set to true is evaluated the difference between
    //  IP and the first hit.
    double deltaParEval(hitXP hit1, hitXP hit2, Eparameters par, bool is0 = false)
    {
      double out = c_over;
      int layer1 = hit1.m_sensorLayer;
      int layer2 = hit2.m_sensorLayer;
      double layerdiff = layer2 - layer1;
      if (layerdiff >= 0 && (layerdiff < 3 || (layer1 == 0 && layer2 == 3))) {
        switch (par) {
          case 0:
            out = abs(hit1.getOmegaEntry() - hit2.getOmegaEntry());
            if (is0) out = abs(hit1.getOmega0() - hit2.getOmegaEntry());
            break;

          case 1:
            out = hit1.getD0Entry() - hit2.getD0Entry();
            if (is0) out = hit1.getD00() - hit2.getD0Entry();
            break;

          case 2:
            out = asin(sin(hit1.getPhi0Entry())) - asin(sin(hit2.getPhi0Entry()));
            if (is0) out = asin(sin(hit1.getPhi00())) - asin(sin(hit2.getPhi0Entry()));
            break;

          case 3:
            out = hit1.getZ0Entry() - hit2.getZ0Entry();
            if (is0) out = hit1.getZ00() - hit2.getZ0Entry();
            break;

          case 4:
            out = hit1.getTanLambdaEntry() - hit2.getTanLambdaEntry();
            if (is0) out = hit1.getTanLambda0() - hit2.getTanLambdaEntry();
            break;
        }
      }
      return out;
    }

//This is the funcion that select the percentage that has to be cut away from
// deltaPar distributions (function of momentum)

    //OLD CUTFUNCTION: NOT USED NOW (1/1000 ACCEPTED)
    double cutFunction(int p, double pwidth)
    {
      double out;
      double mom = p * pwidth;
      if (mom > 0.04)
        out = -7.5 * pow(10, -7) / pow(mom, 3.88) + 1;
      else out = 6.3 * mom + 0.57;
      return out;
    }

    // double cutFunction(int p, double pwid)
    // {
    //   double out;
    //   double mom = p * pwid + c_pmin;
    //   out = -3.971 * pow(10, -7) / pow(mom, 3.373) + 1;
    //   return out;
    // }






  private:

    const double c_pmin = 0.025;
    const double c_pmax = 2.;
    const double c_tmin = 17.*M_PI / 180.; //17 degrees
    const double c_tmax = 5. / 6.*M_PI; //150 degrees
    const int c_nbin = 5000;
    const int c_nbinp = 40;
    const int c_nbinpar = 5;
    const int c_nbinlay = 7;//present IP too
    const int c_nbint = 3;
    double c_pwidth = (c_pmax - c_pmin) / (double)c_nbinp;
    double c_twidth = (c_tmax - c_tmin) / (double)c_nbint;
    const double c_multLimit = 1;
    const int c_over = 9999999;
    int m_pCounter = 0;
    int m_tCounter = 0;
    int m_globCounter = 0;
    bool c_validationON = 0;

    NoKickRTSel m_trackSel;


    TFile* m_outputFile;
    std::vector<double> m_histoLim;
    std::vector<std::vector<std::vector<std::vector<std::vector<TH1F*>>>>> m_histo;
    std::vector<TString> m_namePar = {
      "#omega",
      "d0",
      "#phi0",
      "z0",
      "tan#lambda"
    };

    std::vector<TString> m_unitPar = {
      "[cm^{-1}]",
      "[cm]",
      "[rad]",
      "[cm]",
      ""
    };

  protected:

  };
}
#endif
