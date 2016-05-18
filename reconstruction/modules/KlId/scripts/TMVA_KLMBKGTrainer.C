#include <cstdlib>
#include <iostream>
#include <map>
#include <string>

#include "TChain.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TObjString.h"
#include "TSystem.h"
#include "TROOT.h"

#include "TMVA/Factory.h"
#include "TMVA/Tools.h"
#include "TMVA/TMVAGui.h"
#include "TMVA/Types.h"
#include <TPluginManager.h>


#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <iostream>

using namespace boost::filesystem;

void loadPlugins(const std::string& name);

void TMVA_KLMBKGTrainer(){

	//  -------------------------------------------------------------------------------------
	// OUTPUT ROOT FILE
	// 

	TString outfileName(std::string(std::getenv("BELLE2_LOCAL_DIR"))+"/reconstruction/data/KLMBKGClassifierBDT.root" );
	TFile* outputFile = TFile::Open( outfileName, "RECREATE" );


	TMVA::Factory *factory = new TMVA::Factory( "TMVAFactory", outputFile,
                                            "!V:!Silent:Color:DrawProgressBar:Transformations=:AnalysisType=Classification" );

  factory -> AddVariable("KLMnCluster"        , "KLMnCluster"        , "F");
  factory -> AddVariable("KLMnLayer"          , "KLMnLayer"          , "F");
  factory -> AddVariable("KLMnInnermostlayer" , "KLMnInnermostlayer" , "F");
  factory -> AddVariable("KLMglobalZ"         , "KLMglobalZ"         , "F");
  factory -> AddVariable("KLMtime"            , "KLMtime"            , "F");
  factory -> AddVariable("KLMinvM"            , "KLMinvM"            , "F");
  factory -> AddVariable("KLMtrackDist"       , "KLMtrackDist"       , "F");
  factory -> AddVariable("KLMdistToNextCl"    , "KLMdistToNextCl"    , "F");

//  factory->AddVariable("KLMenergy",                     &m_KLMenergy);

  factory -> AddVariable("KLMshape"                   , "KLMshape"                   , "F");
  factory -> AddVariable("KLMaverageInterClusterDist" , "KLMaverageInterClusterDist" , "F");
  factory -> AddVariable("KLMhitDepth"                , "KLMhitDepth"                , "F");

  // KLM-ECL Vars (ECL clusters that are related to KLM clusters)

  factory -> AddVariable("KLMdistToNextECL"   , "KLMdistToNextECL"   , "F");
  factory -> AddVariable("KLMECLenergy"       , "KLMECLenergy"       , "F");
  factory -> AddVariable("KLMECLE9oE25"       , "KLMECLE9oE25"       , "F");
  factory -> AddVariable("KLMECLtiming"       , "KLMECLtiming"       , "F");
  factory -> AddVariable("KLMECLEerror"       , "KLMECLEerror"       , "F");
  factory -> AddVariable("KLMtrackToECL"      , "KLMtrackToECL"      , "F");

  factory -> AddVariable("KLMECLdeltaL"       , "KLMECLdeltaL"       , "F");
  factory -> AddVariable("KLMECLmintrackDist" , "KLMECLmintrackDist" , "F");





	Double_t signalWeight     = 1.0;
	Double_t backgroundWeight = 1.0;

//-----------------------------------  INPUT FILES HERE  --------------------------------------
// chain needs tree name and the absolute path
	TChain *chain  = new TChain("KLMdata");
	//
	//
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  your path here
	//TODO path to your tuples here
	chain->Add("/afs/desy.de/user/j/jkrohn/nfs/belle2/my_stuff/generation/root_files/reconstruction_comb/*.root");
//----------------------------------------------------------------------------------------------


	factory -> AddSignalTree    ( chain,     signalWeight);
	factory -> AddBackgroundTree( chain,     signalWeight);


	// Apply additional cuts on the signal and background samples (can be different)
	TCut mycutb = "KLMTruth > -2"; // for example: TCut mycuts = "abs(var1)<0.5 && abs(var2-0.5)<1";
	TCut mycuts = "KLMTruth < -1"; // ugly but -2 is atm the value for beambkg //TODO change to nicer output


	factory->PrepareTrainingAndTestTree( mycuts, mycutb,
                                        "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V" );


	factory->BookMethod(TMVA::Types::kBDT, 
				                "KLMBKGClassifierBDT",
                     "NTrees=1000:Shrinkage=0.1!H:!V:IgnoreNegWeightsInTraining:MaxDepth=3:nCuts=256");


	// Train MVAs using the set of training events
	factory->TrainAllMethods();

	// ---- Evaluate all MVAs using the set of test events
	factory->TestAllMethods();

	// ----- Evaluate and compare performance of all configured MVAs
	factory->EvaluateAllMethods();

	// --------------------------------------------------------------

	// Save the output
	outputFile->Close();

	TMVA::TMVAGui( outfileName );

	delete factory;

}


