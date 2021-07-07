/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/dataobjects/FileMetaData.h>
#include <framework/io/RootIOUtilities.h>
#include <framework/io/RootFileInfo.h>
#include <framework/logging/Logger.h>
#include <framework/pcore/Mergeable.h>
#include <framework/core/FileCatalog.h>
#include <framework/utilities/KeyValuePrinter.h>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <TFile.h>
#include <TTree.h>
#include <TBranchElement.h>

#include <iostream>
#include <iomanip>
#include <string>
#include <set>
#include <regex>

using namespace Belle2;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

/** Simple typedef to conveniently define a exp,run,evt structure with a
 * working comparison operator */
using EventInfo = std::tuple<int, int, unsigned int>;

namespace {
  /** Remove the legacy ip information globaltag from the given string of comma separated
   * globaltags */
  std::string removeLegacyGt(const std::string& globaltags)
  {
    std::regex legacy_gt(",?Legacy_IP_Information");
    return std::regex_replace(globaltags, legacy_gt, "");
  }
}

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
  ("no-catalog", "don't register output file in file catalog, This is now the default")
  ("add-to-catalog", "register the output file in the file catalog")
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
the objects in the persistent tree correctly.

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

  B2INFO("Merging files into " << std::quoted(outputfilename));
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
  // map of all mergeable objects found in the persistent tree. The size_t is
  // for counting to make sure we see all objects in all files
  std::map<std::string, std::pair<Mergeable*, size_t>> persistentMergeables;
  // set of all random seeds to print warning on duplicates
  std::set<std::string> allSeeds;
  // set of all users
  std::set<std::string> allUsers;
  // EventInfo for the high/low event numbers of the final FileMetaData
  std::optional<EventInfo> lowEvt, highEvt;
  // set of all branch names in the event tree to compare against to make sure
  // that they're the same in all files
  std::set<std::string> allEventBranches;
  // Release version to compare against. Same as FileMetaData::getRelease() but with the optional -modified removed
  std::string outputRelease;

  // so let's loop over all files and create FileMetaData and merge persistent
  // objects if they inherit from Mergeable, bail if there's something else in
  // there. The idea is that merging the persistent stuff is fast so we catch
  // errors more quickly when we do this as a first step and events later on.
  for (const auto& input : inputfilenames) {
    try {
      RootIOUtilities::RootFileInfo fileInfo(input);
      // Ok, load the FileMetaData from the tree
      const auto &fileMetaData = fileInfo.getFileMetaData();
      // File looks usable, start checking metadata ...
      B2INFO("adding file " << std::quoted(input));
      if(LogSystem::Instance().isLevelEnabled(LogConfig::c_Info)) fileMetaData.Print("all");

      auto branches = fileInfo.getBranchNames();
      if(branches.empty()) {
        throw std::runtime_error("Could not find any branches in event tree");
      }
      if(allEventBranches.empty()) {
        std::swap(allEventBranches,branches);
      }else{
        if(branches!=allEventBranches){
          B2ERROR("Branches in " << std::quoted(input) << " differ from "
              << std::quoted(inputfilenames.front()));
        }
      }

      // File looks good so far, now fix the persistent stuff, i.e. merge all
      // objects in persistent tree
      for(TObject* brObj: *fileInfo.getPersistentTree().GetListOfBranches()){
        auto* br = dynamic_cast<TBranchElement*>(brObj);
        // FileMetaData is handled separately
        if(br && br->GetTargetClass() == FileMetaData::Class() && std::string(br->GetName()) == "FileMetaData")
          continue;
        // Make sure the branch is mergeable
        if(!br || !br->GetTargetClass()->InheritsFrom(Mergeable::Class())){
          B2ERROR("Branch " << std::quoted(br->GetName()) << " in persistent tree not inheriting from Mergable");
          continue;
        }
        // Ok, it's an object we now how to handle so get it from the tree
        Mergeable* object{nullptr};
        br->SetAddress(&object);
        if(br->GetEntry(0)<=0) {
          B2ERROR("Could not read branch " << std::quoted(br->GetName()) << " of entry 0 from persistent tree in "
              << std::quoted(input));
          continue;
        }
        // and either insert it into the map of mergeables or merge with the existing one
        auto it = persistentMergeables.insert(std::make_pair(br->GetName(), std::make_pair(object, 1)));
        if(!it.second) {
          try {
            it.first->second.first->merge(object);
          }catch(std::exception &e){
            B2FATAL("Cannot merge " << std::quoted(br->GetName()) << " in " << std::quoted(input) << ": " << e.what());
          }
          it.first->second.second++;
          // ok, merged, get rid of it.
          delete object;
        }else{
          B2INFO("Found mergeable object " << std::quoted(br->GetName()) << " in persistent tree");
        }
      }

      std::string release = fileMetaData.getRelease();
      if(release == "") {
        B2ERROR("Cannot determine release used to create " <<  std::quoted(input));
        continue;
      }else if(boost::algorithm::ends_with(fileMetaData.getRelease(), "-modified")){
        B2WARNING("File " << std::quoted(input) << " created with modified software "
                  <<  fileMetaData.getRelease()
                  << ": cannot verify that files are compatible");
        release = release.substr(0, release.size() - std::string("-modified").size());
      }

      // so, event tree looks good too. Now we merge the FileMetaData
      if (!outputMetaData) {
        // first input file, just take the event metadata
        outputMetaData = new FileMetaData(fileMetaData);
        outputRelease = release;
      } else {
        // check meta data for consistency, we could move this into FileMetaData...
        if(release != outputRelease) {
          B2ERROR("Release in " << std::quoted(input) << " differs from previous files: " <<
                  fileMetaData.getRelease() << " != " << outputMetaData->getRelease());
        }
        if(fileMetaData.getSteering() != outputMetaData->getSteering()){
          // printing both steering files is not useful for anyone so just throw an error
          B2ERROR("Steering file for " << std::quoted(input) << " differs from previous files.");
        }
        if(fileMetaData.getDatabaseGlobalTag() != outputMetaData->getDatabaseGlobalTag()){
          // Related to BII-6093: we were adding the legacy gt only dependent on input file age, not creation release.
          // This means there is a chance we want to merge files with and without the globaltag added if they cross the
          // boundary. It doesn't hurt to keep the gt but we know we could process some of the files without it so as a remedy we
          // check if the only difference is the legacy gt and if so we remove it from the output metadata ...
          if(removeLegacyGt(fileMetaData.getDatabaseGlobalTag()) == removeLegacyGt(outputMetaData->getDatabaseGlobalTag())) {
            outputMetaData->setDatabaseGlobalTag(removeLegacyGt(outputMetaData->getDatabaseGlobalTag()));
          } else {
            B2ERROR("Database globalTag in " << std::quoted(input) << " differs from previous files: " <<
                    fileMetaData.getDatabaseGlobalTag() << " != " << outputMetaData->getDatabaseGlobalTag());
          }
        }
        if(fileMetaData.getDataDescription() != outputMetaData->getDataDescription()){
          KeyValuePrinter cur(true);
          for (const auto& descrPair : outputMetaData->getDataDescription())
            cur.put(descrPair.first, descrPair.second);
          KeyValuePrinter prev(true);
          for (const auto& descrPair : fileMetaData.getDataDescription())
            prev.put(descrPair.first, descrPair.second);

          B2ERROR("dataDescription in " << std::quoted(input) << " differs from previous files:\n" << cur.string() << " vs.\n" << prev.string());
        }
        if(fileMetaData.isMC() != outputMetaData->isMC()){
          B2ERROR("Type (real/MC) for " << std::quoted(input) << " differs from previous files.");
        }
        // update event numbers ...
        outputMetaData->setMcEvents(outputMetaData->getMcEvents() + fileMetaData.getMcEvents());
        outputMetaData->setNEvents(outputMetaData->getNEvents() + fileMetaData.getNEvents());
      }
      if(fileMetaData.getNEvents() < 1) {
        B2WARNING("File " << std::quoted(input) << " is empty.");
      } else {
        // make sure we have the correct low/high event numbers
        EventInfo curLowEvt = EventInfo{fileMetaData.getExperimentLow(), fileMetaData.getRunLow(), fileMetaData.getEventLow()};
        EventInfo curHighEvt = EventInfo{fileMetaData.getExperimentHigh(), fileMetaData.getRunHigh(), fileMetaData.getEventHigh()};
        if(!lowEvt or curLowEvt < *lowEvt) lowEvt = curLowEvt;
        if(!highEvt or curHighEvt > *highEvt) highEvt = curHighEvt;
      }
      // check if we have seen this random seed already in one of the previous files
      auto it = allSeeds.insert(fileMetaData.getRandomSeed());
      if(!it.second) {
        B2WARNING("Duplicate Random Seed: " << std::quoted(fileMetaData.getRandomSeed()) << " present in more then one file");
      }
      allUsers.insert(fileMetaData.getUser());
      // remember all parent files we encounter
      for (int i = 0; i < fileMetaData.getNParents(); ++i) {
        allParents.insert(fileMetaData.getParent(i));
      }
    }catch(std::exception &e) {
      B2ERROR("input file " << std::quoted(input) << ": " << e.what());
    }
  }

  //Check if the same mergeables were found in all files
  for(const auto &val: persistentMergeables){
    if(val.second.second != inputfilenames.size()){
      B2ERROR("Mergeable " << std::quoted(val.first) << " only present in " << val.second.second << " out of "
              << inputfilenames.size() << " files");
    }
  }

  // Check for user names
  if(allUsers.size()>1) {
      B2WARNING("Multiple different users created input files: " << boost::algorithm::join(allUsers, ", "));
  }

  // Stop processing in case of error
  if (LogSystem::Instance().getMessageCounter(LogConfig::c_Error) > 0) return 1;

  if(!outputMetaData){
      // technically it's rather impossible to arrive here: if there were no
      // input files we exit with a usage message and if any of the files could
      // not be processed then the error count should be >0. Nevertheless
      // let's do this check to be on the very safe side and to make clang
      // analyzer happy.
      B2FATAL("For some reason no files could be processed");
      return 1;
  }
  if(!lowEvt) {
    B2WARNING("All Files were empty");
    lowEvt = EventInfo{-1, -1, 0};
    highEvt = EventInfo{-1, -1, 0};
  }

  // Final changes to metadata
  outputMetaData->setLfn("");
  outputMetaData->setParents(std::vector<std::string>(allParents.begin(), allParents.end()));
  outputMetaData->setLow(std::get<0>(*lowEvt), std::get<1>(*lowEvt), std::get<2>(*lowEvt));
  outputMetaData->setHigh(std::get<0>(*highEvt), std::get<1>(*highEvt), std::get<2>(*highEvt));
  // If more then one file set an empty random seed
  if(inputfilenames.size()>1){
      outputMetaData->setRandomSeed("");
  }
  RootIOUtilities::setCreationData(*outputMetaData);

  // OK we have a valid FileMetaData and merged all persistent objects, now do
  // the conversion of the event trees and create the output file.
  TFile output(outputfilename.c_str(), "RECREATE");
  if (output.IsZombie()) {
    B2ERROR("Could not create output file " << std::quoted(outputfilename));
    return 1;
  }

  TTree* outputEventTree{nullptr};
  for (const auto& input : inputfilenames) {
    B2INFO("processing events from " << std::quoted(input));
    TFile tfile(input.c_str());
    auto* tree = dynamic_cast<TTree*>(tfile.Get("tree"));
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

  // we need to set the LFN to the absolute path name
  outputMetaData->setLfn(fs::absolute(outputfilename, fs::initial_path()).string());
  // and maybe register it in the file catalog
  if(variables.count("add-to-catalog")>0) {
    FileCatalog::Instance().registerFile(outputfilename, *outputMetaData);
  }
  B2INFO("Writing FileMetaData");
  // Create persistent tree
  output.cd();
  TTree outputMetaDataTree("persistent", "persistent");
  outputMetaDataTree.Branch("FileMetaData", &outputMetaData);
  for(auto &it: persistentMergeables){
    outputMetaDataTree.Branch(it.first.c_str(), &it.second.first);
  }
  outputMetaDataTree.Fill();
  outputMetaDataTree.Write();

  // now clean up the mess ...
  for(const auto& val: persistentMergeables){
    delete val.second.first;
  }
  persistentMergeables.clear();
  delete outputMetaData;
  output.Close();
}
