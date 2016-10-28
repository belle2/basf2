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

    SPlot::SPlot(std::string modelFileName, const std::map<std::string, std::vector<float>>& discriminatingVariables) :
      workspace(std::unique_ptr<RooWorkspace>(new RooWorkspace("SPlotWorkspace"))), model(nullptr),
      m_modelObjectName("model"), m_modelYieldsObjectNames( {"signal", "background"}), m_modelPlotComponentNames({"sig", "bkg"})
    {

      modelFile = std::unique_ptr<TFile>(new TFile(modelFileName.c_str(), "UPDATE"));
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
        B2FATAL("SPlot supports only one discriminating variable at the moment, sorry.");
      }

      temp_tree_data = std::unique_ptr<TTree>(new TTree("temp_data_tree", "beschreibung"));
      std::string leaf_name = discriminatingVariables.begin()->first;
      auto vec = discriminatingVariables.begin()->second;

      float current_fit_var;
      temp_tree_data->Branch(leaf_name.c_str(), &current_fit_var, (leaf_name + "/F").c_str());

      for (const auto& x : vec) {
        current_fit_var = x;
        temp_tree_data->Fill();
      }
      discriminating_values = std::unique_ptr<RooDataSet>(new RooDataSet("discriminating_values", "discriminating_values",
                                                          temp_tree_data.get(), vars));

      if (discriminating_values->numEntries() != temp_tree_data->GetEntries()) {
        B2FATAL("RooDataSet threw out some events, probably because they were outside the defined range of the RooRealVar");
      }

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

      sData = std::unique_ptr<RooStats::SPlot>(new RooStats::SPlot("sData", "BASF2 SPlotTeacher", *discriminating_values, model,
                                               *yields));

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

      signal_pdf_weights.resize(numberOfEvents);
      signal_cdf_weights.resize(numberOfEvents);
      background_pdf_weights.resize(numberOfEvents);
      background_cdf_weights.resize(numberOfEvents);
      RooAbsPdf* signal_pdf = workspace->pdf("sig");
      RooAbsPdf* bckgrd_pdf = workspace->pdf("bkg");
      std::string dim = discriminatingVariables.begin()->first;
      RooRealVar v(dim.c_str(), dim.c_str(), 1.8);
      RooArgSet row;
      row.add(v);
      RooAbsReal* signal_cdf = signal_pdf->createCdf(row);
      RooAbsReal* bckgrd_cdf = bckgrd_pdf->createCdf(row);
      // For each event, a weight needs to be retrieved for each class
      auto vector = discriminatingVariables.begin()->second;
      for (int i = 0; i < numberOfEvents; i++) {
        v.setVal(vector[i]);
        signal_cdf_weights[i] = signal_cdf->getVal();
        signal_pdf_weights[i] = signal_pdf->getVal(&row);
        background_cdf_weights[i] = bckgrd_cdf->getVal();
        background_pdf_weights[i] = bckgrd_pdf->getVal(&row);
      }

      TH1* signal_hist = signal_pdf->createHistogram(dim.c_str(), 1000);
      unsigned int nbins = signal_hist->GetNbinsX();

      pdf_binning.resize(nbins + 1);
      for (unsigned int i = 0; i < nbins; ++i) {
        pdf_binning[i] = signal_hist->GetBinLowEdge(i + 1);
      }
      pdf_binning[nbins] = signal_hist->GetBinLowEdge(nbins + 1);

      RooArgSet* bckgrd_vars = bckgrd_pdf->getVariables();
      RooRealVar* xvar = (RooRealVar*) bckgrd_vars->find(dim.c_str());
      TH1* bckgrd_hist = bckgrd_pdf->createHistogram("pdf", *xvar, RooFit::Binning(RooBinning(nbins, &pdf_binning[0])));
      signal_pdf_bins.resize(nbins);
      background_pdf_bins.resize(nbins);
      for (unsigned int i = 0; i < nbins; ++i) {
        signal_pdf_bins[i] = signal_hist->GetBinContent(i + 1);
        background_pdf_bins[i] = bckgrd_hist->GetBinContent(i + 1);
      }

    }

    SPlot::~SPlot()
    {
      // This is usually a memory leak, but ROOT frees the tree somewhere else first ...
      temp_tree_data.release();
    }

    void SPlot::plot(const std::string& prefix, std::string discriminatingVariable)
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

