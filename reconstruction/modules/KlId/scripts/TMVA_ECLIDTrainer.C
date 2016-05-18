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

void TMVA_ECLIDTrainer(){

	//  -------------------------------------------------------------------------------------
	// OUTPUT ROOT FILE
	// 
	// find path to frame work and put training in the corresponding ID folder

	TString outfileName(std::string(std::getenv("BELLE2_LOCAL_DIR"))+"/reconstruction/data/ECLBDT.root" );

	TFile* outputFile = TFile::Open( outfileName, "RECREATE" );

 

	TMVA::Factory *factory = new TMVA::Factory( "TMVAFactory", outputFile,
                                            "!V:!Silent:Color:DrawProgressBar:Transformations=:AnalysisType=Classification" );

	//TODO check order when new vars are available
  factory->AddVariable("ECLenergy"       , "ECLenergy"       , "F");
  factory->AddVariable("ECLE9oE25"       , "ECLE9oE25"       , "F");
  factory->AddVariable("ECLtiming"       , "ECLtiming"       , "F");
  factory->AddVariable("ECLR"            , "ECLR"            , "F");
  factory->AddVariable("ECLEerror"       , "ECLEerror"       , "F");
  factory->AddVariable("ECLdistToTrack"  , "ECLdistToTrack"  , "F");
  factory->AddVariable("ECLdeltaL"       , "ECLdeltaL"       , "F");
  factory->AddVariable("ECLmintrackDist" , "ECLmintrackDist" , "F");



	Double_t signalWeight     = 1.0;
	Double_t backgroundWeight = 1.0;

//-----------------------------------  INPUT FILES HERE  --------------------------------------
// chain needs tree name and the absolute path
	TChain *chain  = new TChain("ECLdata");


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> YOUR PATH HERE

//TODO your path here
	chain->Add("/afs/desy.de/user/j/jkrohn/nfs/belle2/my_stuff/generation/root_files/id_training_data/*.root");
//----------------------------------------------------------------------------------------------


	factory -> AddSignalTree    ( chain,     signalWeight);
	factory -> AddBackgroundTree( chain,     signalWeight);


	// Apply additional cuts on the signal and background samples (can be different)
	TCut mycuts = "ECLTruth > 0"; //
	TCut mycutb = "ECLTruth < 1"; // 


	factory->PrepareTrainingAndTestTree( mycuts, mycutb,
                                        "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V" );


	factory->BookMethod(TMVA::Types::kBDT, 
				                "ECLBDT",
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

