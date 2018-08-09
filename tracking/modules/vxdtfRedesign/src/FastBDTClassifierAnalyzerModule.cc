/*************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2016 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Madlener                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <fstream>
#include <TFile.h>
#include <TTree.h>

#include <tracking/modules/vxdtfRedesign/FastBDTClassifierAnalyzerModule.h>
#include <tracking/spacePointCreation/MapHelperFunctions.h>


namespace Belle2 {
  REG_MODULE(FastBDTClassifierAnalyzer);

  FastBDTClassifierAnalyzerModule::FastBDTClassifierAnalyzerModule() : Module()
  {
    setDescription(
      "analyzes performance of given FastBDT on a test and a training set and determines a global classification cut. TODO");

    addParam("fbdtFileName", m_PARAMfbdtFileName, "file name of the fbdtclassifier");
    addParam("trainSamples", m_PARAMtrainSampleFileName, "filename of the training samples");
    addParam("testSamples", m_PARAMtestSampleFileName, "filename of the test samples");
    addParam("outputFileName", m_PARAMrootOutFileName, "output filename", std::string("FBDTAnalyzer_out.root"));
  }

  void FastBDTClassifierAnalyzerModule::initialize()
  {
    std::ifstream fbdt(m_PARAMfbdtFileName);
    if (!fbdt.is_open()) {
      B2ERROR("Could not open file: " << m_PARAMfbdtFileName << ".");
    }

    std::ifstream train(m_PARAMtrainSampleFileName);
    if (!train.is_open()) {
      B2ERROR("Could not open file: " << m_PARAMtrainSampleFileName << ".");
    }

    std::ifstream test(m_PARAMtestSampleFileName);
    if (!test.is_open()) {
      B2ERROR("Could not open file: " << m_PARAMtestSampleFileName << ".");
    }

    B2DEBUG(1, "Reading Classifier from file: " << m_PARAMfbdtFileName << ".");
    m_classifier.readFromStream(fbdt);
    fbdt.close();
    B2DEBUG(1, "Done");

    B2DEBUG(1, "Reading training samples from file: " << m_PARAMtrainSampleFileName << ".");
    readSamplesFromStream(train, m_trainSample);
    train.close();

    B2DEBUG(1, "Reading training samples from file: " << m_PARAMtestSampleFileName << ".");
    readSamplesFromStream(test, m_testSample);
    test.close();
  }

  void FastBDTClassifierAnalyzerModule::terminate()
  {
    std::ofstream ofs("analyze_trout.dat");
    B2DEBUG(10, "Processing the training sample");
    for (const auto& event : m_trainSample) {
      m_trainOutput.insert(std::make_pair(event.signal, m_classifier.analyze(event.hits)));
      ofs << event.signal << " " << m_classifier.analyze(event.hits) << std::endl;
    }
    ofs.close();

    B2DEBUG(10, "Processing the test sample");
    for (const auto& event : m_testSample) {
      m_testOutput.insert(std::make_pair(event.signal, m_classifier.analyze(event.hits)));
    }

    auto trainBgOut = getValuesToKey(m_trainOutput, 0);
    auto trainSigOut = getValuesToKey(m_trainOutput, 1);

    auto testBgOut = getValuesToKey(m_testOutput, 0);
    auto testSigOut = getValuesToKey(m_testOutput, 1);

    TFile* outfile = new TFile(m_PARAMrootOutFileName.c_str(), "RECREATE");
    TTree* tree = new TTree("classifierOutputs", "outputs of FBDTClassifier for the different samples");
    tree->Branch("train_bg_outputs", &trainBgOut);
    tree->Branch("train_sig_outputs", &trainSigOut);
    tree->Branch("test_bg_outputs", &testBgOut);
    tree->Branch("test_sig_outputs", &testSigOut);

    tree->Fill();
    outfile->cd();
    outfile->Write();
    outfile->Close();
  }
}
