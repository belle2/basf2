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
  m_ChargeMult = iPart->getCharge() * jPart->getCharge();
  m_PPhi  = iPart->getMomentum().Angle(jPart->getMomentum());
  /*
  m_PtDiff = abs(Variable::particlePt(iPart) - Variable::particlePt(jPart));
  m_PzDiff = abs(Variable::particlePz(iPart) - Variable::particlePz(jPart));
  m_TrackD0Diff = abs(Variable::trackD0(iPart) - Variable::trackD0(jPart));
  m_TrackZ0Diff = abs(Variable::trackZ0(iPart) - Variable::trackZ0(jPart));
  m_TrackPValueDiff = abs(Variable::trackPValue(iPart) - Variable::trackPValue(jPart)); // get rid of this eventually
  m_TrackTanLambdaDiff = abs(Variable::trackTanLambda(iPart) - Variable::trackTanLambda(jPart));
  m_TrackPhi0Diff = abs(Variable::trackPhi0(iPart) - Variable::trackPhi0(jPart));
  m_TrackOmegaDiff = abs(Variable::trackOmega(iPart) - Variable::trackOmega(jPart));
  */
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
  // Do I actually need this function?
  updateVariables(iPart, jPart);
  //return {m_PtDiff, m_PzDiff, m_PPhi, m_TrackD0Diff, m_TrackZ0Diff, m_ChargeMult, m_TrackPValueDiff, m_TrackTanLambdaDiff, m_TrackPhi0Diff, m_TrackOmegaDiff,    m_PtDiffEW, m_PzDiffEW, m_TrackD0DiffEW, m_TrackZ0DiffEW, m_TrackTanLambdaDiffEW, m_TrackPhi0DiffEW, m_TrackOmegaDiffEW};
  return {m_PPhi, m_ChargeMult, m_PtDiffEW, m_PzDiffEW, m_TrackD0DiffEW, m_TrackZ0DiffEW, m_TrackTanLambdaDiffEW, m_TrackPhi0DiffEW, m_TrackOmegaDiffEW};
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

    m_TTree -> Branch("PtDiff", &m_PtDiff, "PtDiff/F");
    m_TTree -> Branch("PzDiff", &m_PzDiff, "PzDiff/F");
    m_TTree -> Branch("PPhi"  , &m_PPhi, "PPhi/F");
    m_TTree -> Branch("TrackD0Diff", &m_TrackD0Diff, "TrackD0Diff/F");
    m_TTree -> Branch("TrackZ0Diff", &m_TrackZ0Diff, "TrackZ0Diff/F");
    m_TTree -> Branch("ChargeMult", &m_ChargeMult, "ChargeMult/F");
    m_TTree -> Branch("TrackPValueDiff",   &m_TrackPValueDiff, "TrackPValueDiff/F");
    m_TTree -> Branch("TrackTanLambdaDiff", &m_TrackTanLambdaDiff, "TrackTanLambdaDiff/F");
    m_TTree -> Branch("TrackPhi0Diff", &m_TrackPhi0Diff, "TrackPhi0Diff/F");
    m_TTree -> Branch("TrackOmegaDiff", &m_TrackOmegaDiff, "TrackOmegaDiff/F");

    m_TTree -> Branch("PtDiffEW", &m_PtDiffEW, "PtDiffEW/F");
    m_TTree -> Branch("PzDiffEW", &m_PzDiffEW, "PzDiffEW/F");
    m_TTree -> Branch("TrackD0DiffEW", &m_TrackD0DiffEW, "TrackD0DiffEW/F");
    m_TTree -> Branch("TrackZ0DiffEW", &m_TrackZ0DiffEW, "TrackZ0DiffEW/F");
    m_TTree -> Branch("TrackTanLambdaDiffEW", &m_TrackTanLambdaDiffEW, "TrackTanLambdaDiffEW/F");
    m_TTree -> Branch("TrackPhi0DiffEW", &m_TrackPhi0DiffEW, "TrackPhi0DiffEW/F");
    m_TTree -> Branch("TrackOmegaDiffEW", &m_TrackOmegaDiffEW, "TrackOmegaDiffEW/F");

    m_TTree -> Branch("IsCurl", &m_IsCurl, "IsCurl/F");

    m_target_variable = "IsCurl";
    //m_variables = {"PtDiff", "PzDiff", "PPhi", "TrackD0Diff", "TrackZ0Diff", "ChargeMult", "TrackPValueDiff", "TrackTanLambdaDiff", "TrackPhi0Diff", "TrackOmegaDiff",        "PtDiffEW", "PzDiffEW", "TrackD0DiffEW", "TrackZ0DiffEW","TrackTanLambdaDiffEW", "TrackPhi0DiffEW", "TrackOmegaDiffEW"};
    m_variables = {"PPhi", "ChargeMult", "PtDiffEW", "PzDiffEW", "TrackD0DiffEW", "TrackZ0DiffEW", "TrackTanLambdaDiffEW", "TrackPhi0DiffEW", "TrackOmegaDiffEW"};
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
    MVA::Weightfile::saveToXMLFile(weightfile, "test.xml");
  }
}

float SelectorMVA::getProbability(Particle* iPart, Particle* jPart)
{
  MVA::SingleDataset dataset(m_generalOptions, getVariables(iPart, jPart));
  return m_expert.apply(dataset)[0];
}
