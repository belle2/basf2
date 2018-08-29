/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marcel Hohmann                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/CurlTagger/SelectorMVA.h>

#include <analysis/VariableManager/VertexVariables.h>
#include <analysis/VariableManager/TrackVariables.h>
#include <analysis/VariableManager/Variables.h>

#include <mva/methods/FastBDT.h>
#include <mva/interface/Interface.h>
//#include <mva/interface/Dataset.h> // shouldnt be needed
//#include <mva/interface/Options.h>
//#include <mva/interface/Teacher.h>
//#include <mva/interface/Weightfile.h>

//Root includes
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TMath.h"

using namespace Belle2;
using namespace CurlTagger;

SelectorMVA::SelectorMVA(bool belleFlag, bool trainFlag)
{
  m_TrainFlag = trainFlag;

  if (belleFlag) {
    m_TFileName = "CurlTagger_TrainingData_Belle.root";
    m_identifier = "CurlTagger_FastBDT_Belle";
  } else {
    m_TFileName = "CurlTagger_TrainingData_BelleII.root";
    m_identifier = "CurlTagger_FastBDT_BelleII";
  }
}

SelectorMVA::~SelectorMVA()
{
}

void SelectorMVA::updateVariables(Particle* iPart, Particle* jPart)
{
  if (m_TrainFlag) {
    m_IsCurl = (Variable::genParticleIndex(iPart) == Variable::genParticleIndex(jPart) ? 1 : 0);
  }
  m_PtDiff = abs(Variable::particlePt(iPart) - Variable::particlePt(jPart));
  m_PzDiff = abs(Variable::particlePz(iPart) - Variable::particlePz(jPart));
  m_PPhi   = iPart->getMomentum().Angle(jPart->getMomentum());
  m_D0Diff = abs(Variable::trackD0(iPart) - Variable::trackD0(jPart));
  m_Z0Diff = abs(Variable::trackZ0(iPart) - Variable::trackZ0(jPart));
  m_ChargeMult = iPart->getCharge() * jPart->getCharge();
  m_PvalDiff = abs(iPart->getPValue() - jPart->getPValue());
  // This is defined in TrackVariables.cc but not TrackVariables.h, Intentional?
  //m_TrackTanLambdaDiff = abs(trackTanLambda(iPart) - trackTanLambda(jPart));
}

std::vector<float> SelectorMVA::getVariables(Particle* iPart, Particle* jPart)
{
  // Do I actually need this function?
  updateVariables(iPart, jPart);
  return {m_PtDiff, m_PzDiff, m_PPhi, m_D0Diff, m_Z0Diff, m_ChargeMult, m_PvalDiff};
}

void SelectorMVA::collect(Particle* iPart, Particle* jPart)
{
  updateVariables(iPart, jPart);
  m_TTree -> Fill();
}

void SelectorMVA::initialize()
{
  if (m_TrainFlag) { //make training data
    m_TFile = TFile::Open(m_TFileName.c_str(), "RECREATE");
    m_TTree = new TTree("ntuple", "Training Data for the Curl Tagger MVA");

    m_TTree -> Branch("PtDiff", &m_PtDiff, "PtDiff/F");
    m_TTree -> Branch("PzDiff", &m_PzDiff, "PzDiff/F");
    m_TTree -> Branch("PPhi"  , &m_PPhi, "PPhi/F");
    m_TTree -> Branch("D0Diff", &m_D0Diff, "D0Diff/F");
    m_TTree -> Branch("Z0Diff", &m_Z0Diff, "Z0Diff/F");
    m_TTree -> Branch("ChargeMult", &m_ChargeMult, "ChargeMult/F");
    m_TTree -> Branch("PvalDiff",   &m_PvalDiff, "PvalDiff/F");
    //m_TTree -> Branch("TrackTanLambdaDiff", &m_TrackTanLambdaDiff);

    m_TTree -> Branch("IsCurl", &m_IsCurl, "IsCurl/F");

    m_target_variable = "IsCurl";
    m_variables = {"PtDiff", "PzDiff", "PPhi", "D0Diff", "Z0Diff", "ChargeMult", "PvalDiff"}; // ,"TrackTanLambdaDiff"};
  } else { // normal application
    //load MVA
    auto weightfile = MVA::Weightfile::loadFromDatabase(m_identifier);
    weightfile.getOptions(m_generalOptions);
    m_expert.load(weightfile);
  }
}

void SelectorMVA::finalize()
{
  if (m_TrainFlag) {
    m_TFile -> cd();
    m_TTree -> Write();
    m_TFile -> Write();
    m_TFile -> Close();

    //train MVA
    MVA::GeneralOptions generalOptions;
    generalOptions.m_datafiles = {m_TFileName};
    generalOptions.m_identifier = m_identifier;
    generalOptions.m_variables = m_variables;
    generalOptions.m_target_variable = m_target_variable;
    generalOptions.m_signal_class = 1;
    generalOptions.m_weight_variable = ""; // sets all weights to 1 if blank

    MVA::ROOTDataset dataset(generalOptions);

    MVA::FastBDTOptions specificOptions;
    specificOptions.m_nTrees = 100;
    specificOptions.m_nCuts = 8;
    specificOptions. m_nLevels = 3;

    auto teacher = new MVA::FastBDTTeacher(generalOptions, specificOptions); // does this train it?
    auto weightfile = teacher->train(dataset);
    MVA::Weightfile::saveToDatabase(weightfile, m_identifier);
    MVA::Weightfile::saveToXMLFile(weightfile, "test.xml");
  }
}

float SelectorMVA::getProbability(Particle* iPart, Particle* jPart)
{
  MVA::SingleDataset dataset(m_generalOptions, getVariables(iPart, jPart));
  return m_expert.apply(dataset)[0];
}
