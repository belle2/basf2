#include <tracking/trackFindingCDC/tmva/Teacher.h>

#include <TFile.h>
#include <TCut.h>
#include <TSystem.h>
#include <TMVA/Tools.h>
#include <TMVA/Config.h>
#include <TMVA/Factory.h>
#include <framework/utilities/FileSystem.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Teacher::Teacher()
{
  TMVAUtilities::loadPlugins("FastBDT");
}

void Teacher::createWeights(const std::string& weightFolder, TFile& outputFile, TTree& eventTree,
                            const std::vector<std::string>& variableNames, const std::vector<std::string>& spectatorNames,
                            const TCut& signalCut, const TCut& backgroundCut, const std::string optionString)
{

  TMVA::Tools::Instance();
  TMVA::gConfig().GetIONames().fWeightFileDir = FileSystem::findFile(weightFolder);

  std::string jobName = "TMVAFactory";
  outputFile.cd();
  std::string oldDirectory = gSystem->WorkingDirectory();
  TMVA::Factory factory(jobName, &outputFile, "!V:!Silent:Transformations=I");

  for (const std::string variable : variableNames) {
    factory.AddVariable(variable);
  }

  for (const std::string spectator : spectatorNames) {
    factory.AddSpectator(spectator);
  }

  factory.AddSignalTree(&eventTree);
  factory.AddBackgroundTree(&eventTree);
  factory.PrepareTrainingAndTestTree(signalCut, backgroundCut, "SplitMode=Random");

  factory.BookMethod(TMVA::Types::kPlugins, "FastBDT", "!H:!V:IgnoreNegWeightsInTraining:" + optionString);

  factory.TrainAllMethods();
  factory.TestAllMethods();
  factory.EvaluateAllMethods();

  outputFile.Close();
}
