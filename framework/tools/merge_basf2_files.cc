#include <framework/dataobjects/FileMetaData.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/io/RootIOUtilities.h>
#include <framework/logging/Logger.h>
#include <framework/pcore/Mergeable.h>
#include <framework/core/FileCatalog.h>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <TSystem.h>
#include <TFile.h>
#include <TTree.h>
#include <TBranchElement.h>

#include <iostream>
#include <iomanip>
#include <string>
#include <set>

using namespace   Belle2;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

typedef std::tuple<int, int, unsigned int> EventInfo;

int main(int argc, char* argv[])
{
  // Parse options
  std::string outputfilename;
  std::vector<std::string> inputfilenames;
  po::options_description options("Options");
  options.add_options()
  ("help,h", "print all available options")
  ("output", po::value<std::string>(&outputfilename), "output file name")
  ("file", po::value<std::vector<std::string>>(&inputfilenames), "filename to merge")
  ("force,f", "overwrite existing file")
  ("no-catalog", "don't register output file in file catalog");
  po::positional_options_description positional;
  positional.add("output", 1);
  positional.add("file", -1);
  po::variables_map variables;
  po::store(po::command_line_parser(argc, argv).options(options).positional(positional).run(), variables);
  po::notify(variables);
  if (variables.count("help") || variables.count("output") == 0 || inputfilenames.empty()) {
    std::cout << "Usage: " << argv[0] << " [-f|--force] OUTPUTFILE INPUTFILE [INPUTFILE...]" << std::endl << std::endl;
    std::cout << options << std::endl;
    std::cout << (R"DOC(
This program is intended to merge files created by separate basf2 jobs. It's
similar to hadd but does correctly update the metadata in the file and merges
the objects correctly.

The following restrictions apply:
  - The files have to be created with the same release and steering file
  - The persistent tree is only allowed to contain FileMetaData and objects
    inheriting from Mergeable and the same list of objects needs to be present
    in all files.
  - The event tree needs to contain the same DataStore entries in all files.
)DOC");
    return 1;
  }

  auto logConfig = LogSystem::Instance().getLogConfig();
  for(auto l: {LogConfig::c_Info, LogConfig::c_Warning, LogConfig::c_Error, LogConfig::c_Fatal}){
    logConfig->setLogInfo(l, LogConfig::c_Level | LogConfig::c_Message);
  }

  B2INFO("Merging files into " << boost::io::quoted(outputfilename));
  // check output file
  if (fs::exists(outputfilename) && variables.count("force")==0) {
    B2ERROR("Output file exists, use -f to force overwriting it");
    return 1;
  }
  // check all input files for consistency ...

  // the final metadata we will write out
  FileMetaData* outputMetaData{nullptr};
  // set of all parent LFNs encountered in any file
  std::set<std::string> allParents;
  // map of all mergable objects found in the persistent tree.
  std::map<std::string, std::pair<Mergeable*, size_t>> persistentMergeables;
  // set of all random seeds to print warning on duplicates
  std::set<std::string> allSeeds;
  // EventInfo for the high/low event numbers;
  EventInfo lowEvt{0,0,0}, highEvt{0,0,0};
  // set of all branch names in the event tree to make sure it's the same in
  // all files
  std::set<std::string> allEventBranches;

  // so let's loop over all files and create FileMetaData and merge persistent
  // objects if they inherit from Mergable, bail if there's something else in
  // there. The idea is that merging the persistent stuff is fast so we catch
  // errors more quickly when we do this as a first step and events later on.
  int fileNumber{0};
  for (const auto& input : inputfilenames) {
    ++fileNumber;
    if (!fs::exists(input)) {
      B2ERROR("Input file " << boost::io::quoted(input) << "' does not exist");
      continue;
    }
    TFile tfile(input.c_str());
    if (tfile.IsZombie()) {
      B2ERROR("Could not open '" << boost::io::quoted(input) << "'");
      continue;
    }
    TTree* persistent = dynamic_cast<TTree*>(tfile.Get("persistent"));
    if (!persistent) {
      B2ERROR("No persistent tree found in " <<  boost::io::quoted(input));
      continue;
    }
    if (persistent->GetEntriesFast() != 1) {
      B2ERROR("Found " << persistent->GetEntriesFast() << "!=1 entries in the persistent tree");
      continue;
    }
    FileMetaData* fileMetaData{nullptr};
    persistent->SetBranchAddress("FileMetaData", &fileMetaData);
    if (persistent->GetEntry(0) <= 0) {
      B2ERROR("Problem loading FileMetaData from " << input);
      continue;
    }
    B2INFO("adding file " << boost::io::quoted(input));
    fileMetaData->Print("all");

    // ok now we now that FileMetaData is there, check the branches of the file
    TTree* tree = dynamic_cast<TTree*>(tfile.Get("tree"));
    if(!tree){
      B2ERROR("No event tree found in " << boost::io::quoted(input));
      // we don't need the tree now so no need to continue, we can make additional checks
    }
    std::set<std::string> branches;
    for(TObject* br: *tree->GetListOfBranches()){
      branches.insert(br->GetName());
    }
    if(branches.empty()) {
      B2ERROR("No branches found in event tree in " << boost::io::quoted(input));
      continue;
    }
    if(allEventBranches.empty()) {
      std::swap(allEventBranches,branches);
    }else{
      if(branches!=allEventBranches){
        B2ERROR("Branches in " << boost::io::quoted(input) << " differ from "
                << boost::io::quoted(inputfilenames.front()));
      }
    }

    // File looks good so far, now fixup the persistent stuff, i.e. merge all
    // objects in persistent tree
    for(TObject* brObj: *persistent->GetListOfBranches()){
      TBranchElement* br = dynamic_cast<TBranchElement*>(brObj);
      // FileMetaData is handled separately
      if(br && br->GetTargetClass() == FileMetaData::Class() && std::string(br->GetName()) == "FileMetaData")
        continue;
      // Make sure the branch is mergeable
      if(!br || !br->GetTargetClass()->InheritsFrom(Mergeable::Class())){
        B2ERROR("Branch " << boost::io::quoted(br->GetName()) << " in persistent tree not inheriting from Mergable");
        continue;
      }
      Mergeable* object{nullptr};
      br->SetAddress(&object);
      if(br->GetEntry(0)<=0) {
        B2ERROR("Could not read branch " << boost::io::quoted(br->GetName()) << " of entry 0 from persistent tree in "
                << boost::io::quoted(input));
        continue;
      }
      auto it = persistentMergeables.insert(std::make_pair(br->GetName(), std::make_pair(object, 1)));
      if(!it.second) {
        it.first->second.first->merge(object);
        it.first->second.second++;
      }else{
        B2INFO("Found mergable object " << boost::io::quoted(br->GetName()) << " in persistent tree");
      }
    }

    if (!outputMetaData) {
      // first input, just take the event metadata
      outputMetaData = new FileMetaData(*fileMetaData);
      lowEvt = EventInfo{fileMetaData->getExperimentLow(), fileMetaData->getRunLow(), fileMetaData->getEventLow()};
      highEvt = EventInfo{fileMetaData->getExperimentHigh(), fileMetaData->getRunHigh(), fileMetaData->getEventHigh()};
    } else {
      // check meta data for consistency
      if(fileMetaData->getRelease() != outputMetaData->getRelease()){
        B2ERROR("Release in " << input << " differs from previous files: " <<
                fileMetaData->getRelease() << " != " << outputMetaData->getRelease());
      }
      if(fileMetaData->getSteering() != outputMetaData->getSteering()){
        B2ERROR("Steering in " << input << " differs from previous files: " <<
                fileMetaData->getSteering() << " != " << outputMetaData->getSteering());
      }
      if(fileMetaData->getDatabaseGlobalTag() != outputMetaData->getDatabaseGlobalTag()){
        B2ERROR("Database globalTag in " << input << " differs from previous files: " <<
                fileMetaData->getDatabaseGlobalTag() << " != " << outputMetaData->getDatabaseGlobalTag());
      }
      //FIXME: check user? factor this into a function?
      // merge metadata here ...
      outputMetaData->setMcEvents(outputMetaData->getMcEvents() + fileMetaData->getMcEvents());
      outputMetaData->setNEvents(outputMetaData->getNEvents() + fileMetaData->getNEvents());
      // make sure we have the correct low/high event numbers
      EventInfo curLowEvt = EventInfo{fileMetaData->getExperimentLow(), fileMetaData->getRunLow(), fileMetaData->getEventLow()};
      EventInfo curHighEvt = EventInfo{fileMetaData->getExperimentHigh(), fileMetaData->getRunHigh(), fileMetaData->getEventHigh()};
      if(curLowEvt < lowEvt) std::swap(curLowEvt, lowEvt);
      if(curHighEvt > highEvt) std::swap(curHighEvt, highEvt);
    }
    auto it = allSeeds.insert(fileMetaData->getRandomSeed());
    if(!it.second) {
      B2WARNING("Duplicate Random Seed: " << boost::io::quoted(fileMetaData->getRandomSeed()) << " present in more then one file");
    }
    // remember all parent files we encounter
    for (int i = 0; i < fileMetaData->getNParents(); ++i) {
      allParents.insert(fileMetaData->getParent(i));
    }
  }
  //Check if mergables were found in all files
  for(const auto &val: persistentMergeables){
    if(val.second.second != inputfilenames.size()){
      B2ERROR("Mergeable " << boost::io::quoted(val.first) << " only present in " << val.second.second << " out of "
              << inputfilenames.size() << " files");
    }
  }

  // Final changes to MetaData
  outputMetaData->setLfn("");
  outputMetaData->setParents(std::vector<std::string>(allParents.begin(), allParents.end()));
  outputMetaData->setLow(std::get<0>(lowEvt), std::get<1>(lowEvt), std::get<2>(lowEvt));
  outputMetaData->setHigh(std::get<0>(highEvt), std::get<1>(highEvt), std::get<2>(highEvt));
  outputMetaData->setRandomSeed("");
  RootIOUtilities::setCreationData(*outputMetaData);

  //Stop processing in case of error
  if (LogSystem::Instance().getMessageCounter(LogConfig::c_Error) > 0) return 1;

  //OK we have a valid FileMetaData and merged all persistent objects, now do
  //the conversion of the event trees and create the output file.
  TFile output(outputfilename.c_str(), "RECREATE");
  if (output.IsZombie()) {
    B2ERROR("Could not create output file " << boost::io::quoted(outputfilename));
    return 1;
  }

  TTree* outputEventTree{nullptr};
  for (const auto& input : inputfilenames) {
    B2INFO("processing events from " << boost::io::quoted(input));
    TFile tfile(input.c_str());
    TTree* tree = dynamic_cast<TTree*>(tfile.Get("tree"));
    if(!outputEventTree){
      output.cd();
      outputEventTree = tree->CloneTree(0);
    }else{
      outputEventTree->CopyAddresses(tree);
    }
    // Now let's copy all entries without unpacking (fast), layout the
    // baskets in an optimal order for sequential reading (SortBasketByEntry)
    // and rebuild the index in case some parts of the index are missing
    outputEventTree->CopyEntries(tree, -1, "fast SortBasketsByEntry BuildIndexOnError");
    // and reset the branch addresses to not be connected anymore
    outputEventTree->CopyAddresses(tree, true);
    // finally clean up and close file.
    delete tree;
    tfile.Close();
  }
  // make sure we have an index ...
  if(!outputEventTree->GetTreeIndex()) {
    B2INFO("No Index found: building new index");
    RootIOUtilities::buildIndex(outputEventTree);
  }
  output.cd();
  outputEventTree->Write();
  B2INFO("Done processing events");

  if(variables.count("no-catalog")==0) {
    FileCatalog::Instance().registerFile(outputfilename, *outputMetaData);
  }
  B2INFO("Writing FileMetaData");
  // Create persistent tree
  output.cd();
  TTree outputMetaDataTree("persistent", "persistent");
  outputMetaDataTree.Branch("FileMetaData", &outputMetaData);
  for(auto it: persistentMergeables){
    outputMetaDataTree.Branch(it.first.c_str(), &it.second.first);
  }
  outputMetaDataTree.Fill();
  outputMetaDataTree.Write();

  // now clean up the mess ...
  for(auto val: persistentMergeables){
    delete val.second.first;
  }
  persistentMergeables.clear();
  delete outputMetaData;
  output.Close();
}
