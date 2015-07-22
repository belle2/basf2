/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Lipp                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once


#include <RooFit.h>
#include <RooStats/SPlot.h>
#include <RooDataSet.h>
#include <RooArgList.h>
#include <RooArgSet.h>
#include <RooGaussian.h>
#include <RooExponential.h>
#include <RooChebychev.h>
#include <RooProdPdf.h>
#include <RooAbsPdf.h>
#include <RooAddPdf.h>
#include <RooRealVar.h>
#include <RooAbsArg.h>
#include <RooConstVar.h>
#include <RooWorkspace.h>
#include <Rtypes.h>
#include <RooWorkspace.h>

#include <TIterator.h>
#include <TROOT.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TImage.h>
#include <TAxis.h>

#include <framework/logging/Logger.h>

#include <TTree.h>

#include <string>
#include <memory>
#include <vector>
#include <map>


namespace Belle2 {

  namespace TMVAInterface {

    class SPlot  {
    public:

      /**
       * Constructor
       */
      SPlot(std::string modelFileName, const std::map<std::string, std::vector<float>>& discriminatingVariables);

      /**
       * Destructor
       */
      ~SPlot();

      /**
       * Returns calculated sPlot weights
       */
      std::vector<float> getSPlotWeights() const { return splot_weights; }

      /**
       * Returns calculated CDF values of signal distribution
       */
      std::vector<float> getCDFWeights() const { return cdf_weights; }

      /**
       * Returns calculated PDF values of signal distribution
       */
      std::vector<float> getPDFWeights() const { return pdf_weights; }

      /**
       * Returns calculated binned signal probability S/(S+B)
       */
      std::vector<float> getProbabilityBinned() const { return probability_binned; }

      /**
       * Returns bins of the signal probability S/(S+B)
       */
      std::vector<float> getProbabilityBins() const { return probability_bins; }

      /**
       * Create plot of fitted signal and background yields
       * @param prefix prefix used for the plot filename
       * @param discriminatingVariable which discriminating variable to use
       */
      void plot(std::string prefix, std::string discriminatingVariable);


    private:

      std::unique_ptr<RooWorkspace> workspace;
      std::unique_ptr<RooDataSet> discriminating_values;
      RooAbsPdf* model;
      std::vector<float> splot_weights;
      std::vector<float> cdf_weights;
      std::vector<float> pdf_weights;
      std::vector<float> probability_bins;
      std::vector<float> probability_binned;

      std::string m_modelObjectName; /**< Name of the RooAbsPdf object which represents the model. */
      std::vector<std::string>
      m_modelYieldsObjectNames; /**< Name of the RooRealVar objects that represent the yields of the event classes in the model. */
      std::vector<std::string>
      m_modelPlotComponentNames; /**< Name of RooAbsPdf objects that are part of the model and should be plotted additionally in the control plot. */

      TTree* temp_tree_data; /**< Temporary root tree */
      TFile* modelFile; /**< Model file */

    };
  }

} // Belle2 namespace


