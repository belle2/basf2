/**
 * Thomas Keck 2014
 */
#include "analysis/TMVAMockPlugin/MethodMockPlugin.h"
#include "analysis/TMVAMockPluginInspector/MockPluginInspector.h"

#include <vector>
#include <fstream>

#include "Riostream.h"
#include "TRandom3.h"
#include "TMath.h"
#include "TObjString.h"

#include "TMVA/ClassifierFactory.h"
#include "TMVA/Tools.h"
#include "TMVA/Timer.h"
#include "TMVA/Ranking.h"
#include "TMVA/Results.h"
#include "TMVA/ResultsMulticlass.h"

ClassImp(TMVA::MethodMockPlugin)

TMVA::MethodMockPlugin::MethodMockPlugin(const TString& jobName,
                                         const TString& methodTitle,
                                         DataSetInfo& theData,
                                         const TString& theOption,
                                         TDirectory* theTargetDir) :
  MethodBase(jobName, Types::kPlugins, methodTitle, theData, theOption, theTargetDir)
{
}

TMVA::MethodMockPlugin::MethodMockPlugin(DataSetInfo& theData,
                                         const TString& theWeightFile,
                                         TDirectory* theTargetDir)
  : MethodBase(Types::kPlugins, theData, theWeightFile, theTargetDir)
{
}

TMVA::MethodMockPlugin::~MethodMockPlugin(void)
{
}

Bool_t TMVA::MethodMockPlugin::HasAnalysisType(Types::EAnalysisType, UInt_t, UInt_t)
{
  return kTRUE;
}

void TMVA::MethodMockPlugin::DeclareOptions()
{
}

void TMVA::MethodMockPlugin::ProcessOptions()
{
}

void TMVA::MethodMockPlugin::Init()
{
}


void TMVA::MethodMockPlugin::Reset()
{
}


void TMVA::MethodMockPlugin::Train()
{
  Belle2::MockPluginInspector& inspector = Belle2::MockPluginInspector::GetInstance();
  Data()->SetCurrentType(Types::kTraining);
  UInt_t nEvents = Data()->GetNTrainingEvents();
  UInt_t nFeatures = GetNvar();

  inspector.train_events.clear();
  for (unsigned int iEvent = 0; iEvent < nEvents; ++iEvent) {
    std::vector<float> event(nFeatures + 1, 0);
    for (unsigned int iFeature = 0; iFeature < nFeatures; iFeature++) {
      event[iFeature] = GetTrainingEvent(iEvent)->GetValue(iFeature);
    }
    event[nFeatures] = DataInfo().IsSignal(GetTrainingEvent(iEvent));
    inspector.train_events.push_back(event);
  }

}

void TMVA::MethodMockPlugin::AddWeightsXMLTo(void*) const
{
}

void TMVA::MethodMockPlugin::ReadWeightsFromXML(void*)
{

}

void  TMVA::MethodMockPlugin::ReadWeightsFromStream(std::istream&)
{
}

Double_t TMVA::MethodMockPlugin::GetMvaValue(Double_t*, Double_t*)
{

  Belle2::MockPluginInspector& inspector = Belle2::MockPluginInspector::GetInstance();

  std::vector<float> event(GetNvar(), 0);
  const Event* ev = Data()->GetEvent();
  for (unsigned int iFeature = 0; iFeature < GetNvar(); iFeature++) {
    event[iFeature] = ev->GetValue(iFeature);
  }
  inspector.test_event = event;

  return inspector.mva_value;

}

const TMVA::Ranking* TMVA::MethodMockPlugin::CreateRanking()
{
  return NULL;
}

void TMVA::MethodMockPlugin::GetHelpMessage() const
{
}


