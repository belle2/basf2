/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Valerio Bertacchi                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef NOKICKCUTSEVAL_H
#define NOKICKCUTSEVAL_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <tracking/dataobjects/hitXP.h>
#include <tracking/modules/trackingPerformanceEvaluation/hitXPModule.h>
#include <tracking/modules/vxdtfRedesign/NoKickCutsEvalModule.h>
#include <tracking/trackFindingVXD/sectorMapTools/NoKickCuts.h>
#include <tracking/trackFindingVXD/sectorMapTools/NoKickRTSel.h>
#include <TFile.h>
#include <math.h>


namespace Belle2 {

  /**  This module evaluate the cuts used to select the training sample
  *  of the SectorMap. From simulated data files, during the events create
  *  for each track a vector of hitXP (see hitXP class) with the
  *  first hit on each layer of VXD (see NoKickRTSel class for detalis). Than
  *  fills some histogram with the distribution of difference of parameters
  *  betweeen following layers.
  *  During endrun evaluate the cuts on tails of these distibution using a
  *  funcion that define the percentage that has to be selected. Than these
  *  cuts are fitted in momentum and angular theta distiribution. In the end
  *  the parameters of the fits are put in a matrix (TH3F) on a TFile and
  *  they contains all the informations of the cuts. The output file
  * NoKickCuts.root has to be used duing training of SectorMap.
  */


  class NoKickCutsEvalModule : public Module {
  public:
    NoKickCutsEvalModule();

    virtual ~NoKickCutsEvalModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.   */
    void initialize() override;

    /** Called when entering a new run. */
    void beginRun() override;

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.   */
    void event() override;

    /** This method is called if the current run ends. */
    void endRun() override;

    /** This method is called at the end of the event processing. */
    void terminate() override;

    /** enum for the track-parameters */
    // enum EParameters {
    //   omega,
    //   d0,
    //   phi0,
    //   z0,
    //   tanlambda
    // };

    /**   this method evaluate the difference of a required track parameter "par"
    *   between 2 hit. If is0 is set to true is evaluated the difference between
    *   IP and the first hit.
    * input: (first hit, second hit, track parameter, first hit is IP?)
    * output: Delta(par)
    */
    double deltaParEval(hitXP hit1, hitXP hit2, NoKickCuts::EParameters par, bool is0 = false);


    /** This is the funcion that select the percentage that has to be cut away from
    *  deltaPar distributions (function of momentum)
    *  input: (momentum, widht of momentum bin)
    *  output: (value of the function)
    */
    double cutFunction(int p, double pwidth);

    /** alternative cut function (not used, wider cuts) */
    // double cutFunction(int p, double pwid)
    // {
    //   double out;
    //   double mom = p * pwid + c_pmin;
    //   out = -3.971 * pow(10, -7) / pow(mom, 3.373) + 1;
    //   return out;
    // }

  private:

    const double c_pmin = 0.025; /**< minimum momentum evaluated */
    const double c_pmax = 2.; /**< maximum momentum evaluated */
    const double c_tmin = 17.*M_PI / 180.; /**< 17 degrees. minimum theta evaluated */
    const double c_tmax = 5. / 6.*M_PI; /**< 150 degrees. maximum theta evaluated */
    const int c_nbin = 5000; /**< number of bins of histogram of DeltaX */
    const int c_nbinp = 40; /**< number of momentum bins */
    const int c_nbinpar = 5; /**< number of track parameters */
    const int c_nbinlay = 7;/**< present IP too. number of layers */
    const int c_nbint = 3; /**< number of theta parameters */
    double c_pwidth = (c_pmax - c_pmin) / (double)c_nbinp; /**< width of momentum bin */
    double c_twidth = (c_tmax - c_tmin) / (double)c_nbint; /**< width of theta bin */
    const double c_multLimit = 1; /**< multiplier of the range limit of the histograms of DeltaX */
    const int c_over = 9999999; /**< escape flag of some methods */
    int m_pCounter = 0; /**< conter of hit out of range in momentum */
    int m_tCounter = 0; /**< counter of hit out of range in theta */
    int m_globCounter = 0; /**< counter of tracks cutted from global cuts */
    bool c_validationON; /**< flag to activate some validation plots */
    bool c_fitMethod; /**< flag to activate the fit method to evaluate the cuts */

    NoKickRTSel m_trackSel; /**< auxiliary variable to use methods of NoKickRTSel */
    TFile* m_outputFile; /**< output file of cuts */
    std::vector<double> m_histoLim; /**< limits of DeltaX histograms */
    std::vector<std::vector<std::vector<std::vector<std::vector<TH1F*>>>>> m_histo; /**< DeltaX histograms */

    std::vector<TString> m_namePar = { /**< name of track parameters */
      "#omega",
      "d0",
      "#phi0",
      "z0",
      "tan#lambda"
    };

    std::vector<TString> m_unitPar = { /**< units of tracks parameters */
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
