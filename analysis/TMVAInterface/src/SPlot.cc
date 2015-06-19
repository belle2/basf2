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

namespace Belle2 {

  namespace TMVAInterface {

    SPlot::SPlot(std::string modelFileName, const std::map<std::string, std::vector<float>>& discriminatingVariables) : model(nullptr),
      m_modelObjectName("model"), m_modelYieldsObjectNames( {"signal", "background"}), m_modelPlotComponentNames({"sig", "bkg"})
    {

      workspace = std::make_shared<RooWorkspace>("SPlotWorkspace");
      TFile modelFile(modelFileName.c_str());

      // Check if it's derived from RooAbsPdf
      model = dynamic_cast<RooAbsPdf*>(modelFile.Get(m_modelObjectName.c_str()));
      if (!model) {
        B2FATAL("SPlot: The object " << m_modelObjectName << " is either not present in the file " << modelFileName <<
                " or not derived from RooAbsPdf.");
      }
      workspace->import(*model);

      // The reason for extracting the variable from the model is, that this way,
      // the range and binning is just as the user set it.
      RooArgSet vars;
      for (auto& variable : discriminatingVariables) {
        RooRealVar* variableInModel = new RooRealVar(*workspace->var(variable.first.c_str()), variable.first.c_str());
        vars.add(*variableInModel);
      }
      // for each discriminating variable, remember the values, as this information is needed within the sPlot algorithm

      discriminating_values = std::make_shared<RooDataSet>("discriminating_values", "discriminating_values", vars);

      int numberOfEvents = discriminatingVariables.begin()->second.size();
      for (int i = 0; i < numberOfEvents; ++i) {
        RooArgSet row;
        for (auto& variable : discriminatingVariables) {
          RooRealVar v(variable.first.c_str(), variable.first.c_str(), variable.second[i]);
          row.add(v);
        }
        discriminating_values->add(row);
      }

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

      cdf_weights.resize(numberOfEvents);
      RooAbsPdf* signal_pdf = workspace->pdf("sig");
      std::string dim = discriminatingVariables.begin()->first;
      RooRealVar v(dim.c_str(), dim.c_str(), 1.8);
      RooArgSet row;
      row.add(v);
      RooAbsReal* signal_cdf = signal_pdf->createCdf(row);
      // For each event, a weight needs to be retrieved for each class
      auto vector = discriminatingVariables.begin()->second;
      for (int i = 0; i < numberOfEvents; i++) {
        v.setVal(vector[i]);
        //REgularisation of cdf values
        cdf_weights[i] = signal_cdf->getVal() * 0.8 + 0.1;
      }

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

