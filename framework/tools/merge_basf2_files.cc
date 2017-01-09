#include <framework/dataobjects/FileMetaData.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/io/RootIOUtilities.h>
#include <framework/logging/Logger.h>
#include <framework/pcore/Mergeable.h>
#include <framework/core/FileCatalog.h>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

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

/** Simple typedef to conveniently define a exp,run,evt structure with a
 * working comparison operator */
typedef std::tuple<int, int, unsigned int> EventInfo;

int main(int argc, char* argv[])
{
  // Parse options
  std::string outputfilename;
  std::vector<std::string> inputfilenames;
  po::options_description options("Options");
  options.add_options()
  ("help,h", "print all available options")
  ("output,o", po::value<std::string>(&outputfilename), "output file name")
  ("file", po::value<std::vector<std::string>>(&inputfilenames), "filename to merge")
  ("force,f", "overwrite existing file")
  ("no-catalog", "don't register output file in file catalog")
  ("quiet,q", "if given don't print infos, just warnings and errors");
  po::positional_options_description positional;
  positional.add("output", 1);
  positional.add("file", -1);
  po::variables_map variables;
  po::store(po::command_line_parser(argc, argv).options(options).positional(positional).run(), variables);
  po::notify(variables);
  if (variables.count("help") || variables.count("output") == 0 || inputfilenames.empty()) {
    std::cout << "Usage: " << argv[0] << " [<options>] OUTPUTFILE INPUTFILE [INPUTFILE...]" << std::endl;
    std::cout << "       " << argv[0] << " [<options>] [--file INPUTFILE...] "
              << "-o OUTPUTFILE [--file INPUTFILE...]"  << std::endl << std::endl;
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

  // Remove the {module:} from log messages
  auto logConfig = LogSystem::Instance().getLogConfig();
  for(auto l: {LogConfig::c_Info, LogConfig::c_Warning, LogConfig::c_Error, LogConfig::c_Fatal}){
    logConfig->setLogInfo(l, LogConfig::c_Level | LogConfig::c_Message);
  }
  if(variables.count("quiet")>0){
      logConfig->setLogLevel(LogConfig::c_Warning);
  }

  B2INFO("Merging files into " << boost::io::quoted(outputfilename));
  // check output file
  if (fs::exists(outputfilename) && variables.count("force")==0) {
    B2ERROR("Output file exists, use -f to force overwriting it");
    return 1;
  }
  // First we check all input files for consistency ...

  // the final metadata we will write out
  FileMetaData* outputMetaData{nullptr};
  // set of all parent LFNs encountered in any file
  std::set<std::string> allParents;
  // map of all mergable objects found in the persistent tree. The size_t is
  // for counting to make sure we see all objects in all files
  std::map<std::string, std::pair<Mergeable*, size_t>> persistentMergeables;
  // set of all random seeds to print warning on duplicates
  std::set<std::string> allSeeds;
  // set of all users
  std::set<std::string> allUsers;
  // EventInfo for the high/low event numbers of the final FileMetaData
  EventInfo lowEvt{0,0,0}, highEvt{0,0,0};
  // set of all branch names in the event tree to compare against to make sure
  // that they're the same in all files
  std::set<std::string> allEventBranches;

  // so let's loop over all files and create FileMetaData and merge persistent
  // objects if they inherit from Mergable, bail if there's something else in
  // there. The idea is that merging the persistent stuff is fast so we catch
  // errors more quickly when we do this as a first step and events later on.
  for (const auto& input : inputfilenames) {
    if (!fs::exists(input)) {
      B2ERROR("Input file " << boost::io::quoted(input) << " does not exist");
      continue;
    }
    // it exists but can we open it?
    TFile tfile(input.c_str());
    if (tfile.IsZombie()) {
      B2ERROR("Could not open " << boost::io::quoted(input));
      continue;
    }
    // get the persistent tree in the file
    TTree* persistent = dynamic_cast<TTree*>(tfile.Get("persistent"));
    if (!persistent) {
      B2ERROR("No persistent tree found in " <<  boost::io::quoted(input));
      continue;
    }
    // if some used hadd or did something else strange we might have more or
    // less then one entry in the persistent tree
    if (persistent->GetEntriesFast() != 1) {
      B2ERROR("Found " << persistent->GetEntriesFast() << "!=1 entries in the persistent tree in "
              << boost::io::quoted(input));
      continue;
    }
    // Ok, load the FileMetaData from the tree
    FileMetaData* fileMetaData{nullptr};
    persistent->SetBranchAddress("FileMetaData", &fileMetaData);
    if (persistent->GetEntry(0) <= 0) {
      B2ERROR("Problem loading FileMetaData from " << input);
      continue;
    }
    // File looks useable, start checking metadata ...
    B2INFO("adding file " << boost::io::quoted(input));
    if(LogSystem::Instance().isLevelEnabled(LogConfig::c_Info)) fileMetaData->Print("all");

    // ok now we now that FileMetaData is there, check the branches of the file
    // and make sure they are all equal
    TTree* tree = dynamic_cast<TTree*>(tfile.Get("tree"));
    if(!tree){
      B2ERROR("No event tree found in " << boost::io::quoted(input));
      continue;
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
      // Ok, it's an object we now how to handle so get it from the tree
      Mergeable* object{nullptr};
      br->SetAddress(&object);
      if(br->GetEntry(0)<=0) {
        B2ERROR("Could not read branch " << boost::io::quoted(br->GetName()) << " of entry 0 from persistent tree in "
                << boost::io::quoted(input));
        continue;
      }
      // and either insert it into the map of mergeables or merge with the existing one
      auto it = persistentMergeables.insert(std::make_pair(br->GetName(), std::make_pair(object, 1)));
      if(!it.second) {
        it.first->second.first->merge(object);
        it.first->second.second++;
      }else{
        B2INFO("Found mergable object " << boost::io::quoted(br->GetName()) << " in persistent tree");
      }
    }

    // so, event tree looks good too. Now we merge the FileMetaData
    if (!outputMetaData) {
      // first input file, just take the event metadata
      outputMetaData = new FileMetaData(*fileMetaData);
      lowEvt = EventInfo{fileMetaData->getExperimentLow(), fileMetaData->getRunLow(), fileMetaData->getEventLow()};
      highEvt = EventInfo{fileMetaData->getExperimentHigh(), fileMetaData->getRunHigh(), fileMetaData->getEventHigh()};
    } else {
      // check meta data for consistency, we could move this into FileMetaData...
      if(fileMetaData->getRelease() != outputMetaData->getRelease()){
        B2ERROR("Release in " << boost::io::quoted(input) << " differs from previous files: " <<
                fileMetaData->getRelease() << " != " << outputMetaData->getRelease());
      }
      if(fileMetaData->getSteering() != outputMetaData->getSteering()){
        // printing both steering files is not useful for anyone so just throw an error
        B2ERROR("Steering file for " << boost::io::quoted(input) << " differs from previous files.");
      }
      if(fileMetaData->getDatabaseGlobalTag() != outputMetaData->getDatabaseGlobalTag()){
        B2ERROR("Database globalTag in " << boost::io::quoted(input) << " differs from previous files: " <<
                fileMetaData->getDatabaseGlobalTag() << " != " << outputMetaData->getDatabaseGlobalTag());
      }
      // update event numbers ...
      outputMetaData->setMcEvents(outputMetaData->getMcEvents() + fileMetaData->getMcEvents());
      outputMetaData->setNEvents(outputMetaData->getNEvents() + fileMetaData->getNEvents());
      // make sure we have the correct low/high event numbers
      EventInfo curLowEvt = EventInfo{fileMetaData->getExperimentLow(), fileMetaData->getRunLow(), fileMetaData->getEventLow()};
      EventInfo curHighEvt = EventInfo{fileMetaData->getExperimentHigh(), fileMetaData->getRunHigh(), fileMetaData->getEventHigh()};
      if(curLowEvt < lowEvt) std::swap(curLowEvt, lowEvt);
      if(curHighEvt > highEvt) std::swap(curHighEvt, highEvt);
    }
    // check if we have seen this random seed already in one of the previous files
    auto it = allSeeds.insert(fileMetaData->getRandomSeed());
    if(!it.second) {
      B2WARNING("Duplicate Random Seed: " << boost::io::quoted(fileMetaData->getRandomSeed()) << " present in more then one file");
    }
    allUsers.insert(fileMetaData->getUser());
    // remember all parent files we encounter
    for (int i = 0; i < fileMetaData->getNParents(); ++i) {
      allParents.insert(fileMetaData->getParent(i));
    }
  }

  //Check if the same mergables were found in all files
  for(const auto &val: persistentMergeables){
    if(val.second.second != inputfilenames.size()){
      B2ERROR("Mergeable " << boost::io::quoted(val.first) << " only present in " << val.second.second << " out of "
              << inputfilenames.size() << " files");
    }
  }

  // Check for user names
  if(allUsers.size()>1) {
      B2WARNING("Multiple different users created input files: " << boost::algorithm::join(allUsers, ", "));
  }

  // Stop processing in case of error
  if (LogSystem::Instance().getMessageCounter(LogConfig::c_Error) > 0) return 1;

  // Final changes to MetaData
  outputMetaData->setLfn("");
  outputMetaData->setParents(std::vector<std::string>(allParents.begin(), allParents.end()));
  outputMetaData->setLow(std::get<0>(lowEvt), std::get<1>(lowEvt), std::get<2>(lowEvt));
  outputMetaData->setHigh(std::get<0>(highEvt), std::get<1>(highEvt), std::get<2>(highEvt));
  // If more then one file set an empty random seed
  if(inputfilenames.size()>1){
      outputMetaData->setRandomSeed("");
  }
  RootIOUtilities::setCreationData(*outputMetaData);

  // OK we have a valid FileMetaData and merged all persistent objects, now do
  // the conversion of the event trees and create the output file.
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
  // and finally write the tree
  output.cd();
  outputEventTree->Write();
  B2INFO("Done processing events");

  // add it to the file catalog. This also modifies the LFN in the FileMetaData
  // so we do it before writing the persistent tree
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
