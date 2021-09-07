/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/CurlTagger/SelectorMVA.h>

#include <analysis/variables/TrackVariables.h>
#include <analysis/variables/MCTruthVariables.h>
#include <analysis/variables/Variables.h>

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

SelectorMVA::~SelectorMVA() = default;

void SelectorMVA::updateVariables(Particle* iPart, Particle* jPart)
{
  if (m_TrainFlag) {
    m_IsCurl = (Variable::genParticleIndex(iPart) == Variable::genParticleIndex(jPart) ? 1 : 0);
  }
  m_ChargeProduct = iPart->getCharge() * jPart->getCharge();
  m_PPhi  = acos(iPart->getMomentum().Unit().Dot(jPart->getMomentum().Unit()));
  m_PtDiffEW = abs(Variable::particlePt(iPart) - Variable::particlePt(jPart)) / (Variable::particlePtErr(
                 iPart) + Variable::particlePtErr(jPart));
  m_PzDiffEW = abs(Variable::particlePz(iPart) - Variable::particlePz(jPart)) / (Variable::particlePzErr(
                 iPart) + Variable::particlePzErr(jPart));
  m_TrackD0DiffEW = abs(Variable::trackD0(iPart) - Variable::trackD0(jPart)) / (Variable::trackD0Error(
                      iPart) + Variable::trackD0Error(jPart));
  m_TrackZ0DiffEW = abs(Variable::trackZ0(iPart) - Variable::trackZ0(jPart)) / (Variable::trackZ0Error(
                      iPart) + Variable::trackZ0Error(jPart));
  m_TrackTanLambdaDiffEW = abs(Variable::trackTanLambda(iPart) - Variable::trackTanLambda(jPart)) / (Variable::trackTanLambdaError(
                             iPart) + Variable::trackTanLambdaError(jPart));
  m_TrackPhi0DiffEW = abs(Variable::trackPhi0(iPart) - Variable::trackPhi0(jPart)) / (Variable::trackPhi0Error(
                        iPart) + Variable::trackPhi0Error(jPart));
  m_TrackOmegaDiffEW = abs(Variable::trackOmega(iPart) - Variable::trackOmega(jPart)) / (Variable::trackOmegaError(
                         iPart) + Variable::trackOmegaError(jPart));
}

std::vector<float> SelectorMVA::getVariables(Particle* iPart, Particle* jPart)
{
  updateVariables(iPart, jPart);
  return {m_PPhi, m_ChargeProduct, m_PtDiffEW, m_PzDiffEW, m_TrackD0DiffEW, m_TrackZ0DiffEW, m_TrackTanLambdaDiffEW, m_TrackPhi0DiffEW, m_TrackOmegaDiffEW};
}

void SelectorMVA::collectTrainingInfo(Particle* iPart, Particle* jPart)
{
  updateVariables(iPart, jPart);
  m_TTree -> Fill();
}

void SelectorMVA::initialize()
{
  if (m_TrainFlag) { //make training data
    m_TFile = TFile::Open(m_TFileName.c_str(), "RECREATE");
    m_TTree = new TTree("ntuple", "Training Data for the Curl Tagger MVA");

    m_TTree -> Branch("PPhi"  , &m_PPhi, "PPhi/F");
    m_TTree -> Branch("ChargeProduct", &m_ChargeProduct, "ChargeProduct/F");
    m_TTree -> Branch("PtDiffEW", &m_PtDiffEW, "PtDiffEW/F");
    m_TTree -> Branch("PzDiffEW", &m_PzDiffEW, "PzDiffEW/F");
    m_TTree -> Branch("TrackD0DiffEW", &m_TrackD0DiffEW, "TrackD0DiffEW/F");
    m_TTree -> Branch("TrackZ0DiffEW", &m_TrackZ0DiffEW, "TrackZ0DiffEW/F");
    m_TTree -> Branch("TrackTanLambdaDiffEW", &m_TrackTanLambdaDiffEW, "TrackTanLambdaDiffEW/F");
    m_TTree -> Branch("TrackPhi0DiffEW", &m_TrackPhi0DiffEW, "TrackPhi0DiffEW/F");
    m_TTree -> Branch("TrackOmegaDiffEW", &m_TrackOmegaDiffEW, "TrackOmegaDiffEW/F");

    m_TTree -> Branch("IsCurl", &m_IsCurl, "IsCurl/F");

    m_target_variable = "IsCurl";
    m_variables = {"PPhi", "ChargeProduct", "PtDiffEW", "PzDiffEW", "TrackD0DiffEW", "TrackZ0DiffEW", "TrackTanLambdaDiffEW", "TrackPhi0DiffEW", "TrackOmegaDiffEW"};

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
    specificOptions.m_nTrees = 1000;
    //specificOptions.m_shrinkage = 0.10;
    specificOptions.m_nCuts = 16;
    specificOptions. m_nLevels = 4;

    auto teacher = new MVA::FastBDTTeacher(generalOptions, specificOptions);
    auto weightfile = teacher->train(dataset);
    MVA::Weightfile::saveToDatabase(weightfile, m_identifier);
    //MVA::Weightfile::saveToXMLFile(weightfile, "test.xml");
  }
}

float SelectorMVA::getResponse(Particle* iPart, Particle* jPart)
{
  MVA::SingleDataset dataset(m_generalOptions, getVariables(iPart, jPart));
  return m_expert.apply(dataset)[0];
}
