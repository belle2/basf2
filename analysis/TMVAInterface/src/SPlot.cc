/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Lipp                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/TMVAInterface/SPlot.h>
#include "TH1.h"

namespace Belle2 {

  namespace TMVAInterface {

    SPlot::SPlot(std::string modelFileName, const std::map<std::string, std::vector<float>>& discriminatingVariables) : model(nullptr),
      m_modelObjectName("model"), m_modelYieldsObjectNames( {"signal", "background"}), m_modelPlotComponentNames({"sig", "bkg"})
    {

      workspace = std::unique_ptr<RooWorkspace>(new RooWorkspace("SPlotWorkspace"));
      modelFile = new TFile(modelFileName.c_str(), "UPDATE");
      modelFile->cd();

      // Check if it's derived from RooAbsPdf
      model = dynamic_cast<RooAbsPdf*>(modelFile->Get(m_modelObjectName.c_str()));
      if (!model) {
        B2FATAL("SPlot: The object " << m_modelObjectName << " is either not present in the file " << modelFileName <<
                " or not derived from RooAbsPdf.");
      }
      workspace->import(*model);

      // The reason for extracting the variable from the model is, that this way,
      // the range and binning is just as the user set it.
      RooArgSet vars;
      for (auto& variable : discriminatingVariables) {
        vars.add(*workspace->var(variable.first.c_str()), variable.first.c_str());
      }

      // We only support one discriminating variable at the moment
      if (discriminatingVariables.size() != 1) {
        B2FATAL("SPlot supports only one discriminating variable at the moment, sorry.")
      }

      temp_tree_data = new TTree("temp_data_tree", "beschreibung");
      std::string leaf_name = discriminatingVariables.begin()->first;
      auto vec = discriminatingVariables.begin()->second;

      float current_fit_var;
      temp_tree_data->Branch(leaf_name.c_str(), &current_fit_var, (leaf_name + "/F").c_str());

      for (const auto& x : vec) {
        current_fit_var = x;
        temp_tree_data->Fill();
      }
      discriminating_values = std::unique_ptr<RooDataSet>(new RooDataSet("discriminating_values", "discriminating_values", temp_tree_data,
                                                          vars));

      int numberOfEvents = vec.size();

      auto yields = new RooArgList("yields");
      for (auto& yield : m_modelYieldsObjectNames) {
        std::cout << "SPlot: Set range of yield variable " << yield << std::endl;
        workspace->var(yield.c_str())->setMin(0.0);
        workspace->var(yield.c_str())->setMax(numberOfEvents);
        workspace->var(yield.c_str())->setVal(workspace->var(yield.c_str())->getVal() * numberOfEvents);

        RooRealVar* variableInModel = static_cast<RooRealVar*>(workspace->var(yield.c_str()));
        yields->add(*variableInModel);
      }

      model = static_cast<RooAddPdf*>(workspace->pdf(m_modelObjectName.c_str()));

      // fit the model to the data.
      std::cout << "SPlot: Fit data to model" << std::endl;
      model->Print("t");
      model->fitTo(*discriminating_values, RooFit::Extended());

      std::cout << "SPlot: number of entries: " << numberOfEvents << std::endl;

      RooStats::SPlot* sData = new RooStats::SPlot("sData", "BASF2 SPlotTeacher", *discriminating_values, model, *yields);

      RooArgList sWeightedVars = sData->GetSWeightVars();
      std::cout << "For the following " << sWeightedVars.getSize() << " classes sWeights were calculated: ";
      for (Int_t i = 0; i < sWeightedVars.getSize(); i++) {
        std::cout << sWeightedVars.at(i)->GetName();
        if (i != sWeightedVars.getSize() - 1) {
          std::cout << ", ";
        }
      }
      std::cout << "." << std::endl;

      if (sWeightedVars.getSize() != 2) {
        B2FATAL("TMVASPlotTeacher: The SPlot algorithm did not succeed, the number of classes for which sWeights were calculated doesn't match with the number of classes.");
      }

      splot_weights.resize(numberOfEvents);
      // For each event, a weight needs to be retrieved for each class
      for (int i = 0; i < numberOfEvents; i++) {
        splot_weights[i] = sData->GetSWeight(i, yields->at(0)->GetName());
      }

      pdf_weights.resize(numberOfEvents);
      cdf_weights.resize(numberOfEvents);
      RooAbsPdf* signal_pdf = workspace->pdf("sig");
      RooAbsPdf* bckgrd_pdf = workspace->pdf("bkg");
      std::string dim = discriminatingVariables.begin()->first;
      RooRealVar v(dim.c_str(), dim.c_str(), 1.8);
      RooArgSet row;
      row.add(v);
      RooAbsReal* signal_cdf = signal_pdf->createCdf(row);
      // For each event, a weight needs to be retrieved for each class
      auto vector = discriminatingVariables.begin()->second;
      for (int i = 0; i < numberOfEvents; i++) {
        v.setVal(vector[i]);
        cdf_weights[i] = signal_cdf->getVal();
        pdf_weights[i] = signal_pdf->getVal(&row);
      }

      TH1* signal_hist = signal_pdf->createHistogram(dim.c_str(), 1000);
      TH1* bckgrd_hist = bckgrd_pdf->createHistogram(dim.c_str(), 1000);
      bckgrd_hist->Add(signal_hist);
      signal_hist->Divide(bckgrd_hist);

      unsigned int nbins = signal_hist->GetNbinsX();
      probability_bins.resize(nbins + 1);
      probability_binned.resize(nbins);
      for (unsigned int i = 0; i < signal_hist->GetNbinsX(); ++i) {
        probability_bins[i] = signal_hist->GetBinLowEdge(i + 1);
        probability_binned[i] = signal_hist->GetBinContent(i + 1);
      }
      probability_bins[nbins] = signal_hist->GetBinLowEdge(nbins + 1);

    }

    SPlot::~SPlot()
    {
      discriminating_values.reset();
      delete temp_tree_data;
      modelFile->Close();
      delete modelFile;
    }

    void SPlot::plot(std::string prefix, std::string discriminatingVariable)
    {

      RooRealVar* discriminating_variable = workspace->var(discriminatingVariable.c_str());
      std::cout << "SPlot: Generating plot for discriminating variable " << discriminatingVariable.c_str() << "." << std::endl;

      // Set ROOT batch mode to true, so it does not show the GUI
      // while the fit is plotted.
      bool batchMode = gROOT->IsBatch();
      bool batchModeChanged = false;
      if (batchMode == kFALSE) {
        std::cout << "TMVASPlotTeacher: ROOT batch mode is set to kFALSE, setting to kTRUE." << std::endl;
        gROOT->SetBatch(kTRUE);
        batchModeChanged = true;
      }

      RooPlot* xframe = discriminating_variable->frame(RooFit::Title("Fit of provided model"));
      auto canvas = new TCanvas("splot_fit", "splot_fit");

      discriminating_values->plotOn(xframe);
      model->plotOn(xframe);

      // Plot components of the model
      for (auto& component : m_modelPlotComponentNames) {
        std::cout << "TMVASPlotTeacher: Plotting model component " << component << " in addition to the model." << std::endl;
        model->plotOn(xframe, RooFit::Components(*(workspace->pdf(component.c_str()))), RooFit::LineStyle(kDashed)) ;
      }
      xframe->Draw();

      std::string pngFileName = prefix + discriminatingVariable + "_pre_splot_fit.png";
      TImage* img = TImage::Create();
      img->FromPad(canvas);
      img->WriteImage(pngFileName.c_str());

      // Reset ROOT batch mode to the previous value.
      if (batchModeChanged) {
        gROOT->SetBatch(batchMode);
        std::cout << "TMVASPlotTeacher: Setting ROOT batch mode back to kFALSE" << std::endl;
      }
    }
  }

} // Belle2 namespace

