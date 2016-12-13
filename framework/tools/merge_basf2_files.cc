#include <framework/dataobjects/FileMetaData.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <TSystem.h>
#include <TFile.h>
#include <TTree.h>

#include <iostream>
#include <string>
#include <set>

using namespace   Belle2;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

int main(int argc, char* argv[])
{
  std::string outputfilename;
  std::vector<std::string> inputfilenames;
  po::options_description options("Options");
  options.add_options()
  ("help,h", "print all available options")
  ("output", po::value<std::string>(&outputfilename), "output file name")
  ("file", po::value<std::vector<std::string>>(&inputfilenames), "filename to merge")
  ("force,f", "overwrite existing file");

  po::positional_options_description positional;
  positional.add("output", 1);
  positional.add("file", -1);
  po::variables_map variables;
  po::store(po::command_line_parser(argc, argv).options(options).positional(positional).run(), variables);
  po::notify(variables);
  if (variables.count("help") || variables.count("output") == 0 || inputfilenames.size() < 1) {
    std::cout << "Usage: " << argv[0] << "[OPTIONS] OUTPUTFILE INPUTFILE1 INPUTFILE2 [INPUTFILE...]" << std::endl;
    std::cout << options << std::endl;
    return 1;
  }
  bool overwrite = variables.count("force") > 0;
  B2INFO("Merging files into " << outputfilename);
  // check output file
  if (fs::exists(outputfilename) && !overwrite) {
    B2ERROR("Output file exists, use -f to force overwriting it");
    return 1;
  }
  TFile output(outputfilename.c_str(), "RECREATE");
  if (output.IsZombie()) {
    B2ERROR("Could not create output file");
    return 1;
  }
  // check all input files
  std::vector<TFile*> inputs;
  for (const auto& input : inputfilenames) {
    if (!fs::exists(input)) {
      B2ERROR("Input file '" << input << "' does not exist");
      continue;
    }
    TFile tfile(input.c_str());
    if (tfile.IsZombie()) {
      B2ERROR("Could not open '" << input << "'");
    }
    TTree* persistent = dynamic_cast<TTree*>(tfile.Get("persistent"));
    TTree* tree = dynamic_cast<TTree*>(tfile.Get("tree"));
    if (!tree) {
      B2ERROR("No event tree found in " << input);
    }
    if (!persistent) {
      B2ERROR("No persistent tree found in " << input);
      continue;
    }
    if (persistent->GetEntriesFast() != 1) {
      B2ERROR("Found " << tree->GetEntriesFast() << " entries in the persistent tree");
      continue;
    }
    // FIXME: and add checks for user, steering file, random seeds, and so
    // forth before actually starting to create the file
  }
  if (LogSystem::Instance().getMessageCounter(LogConfig::c_Error) > 0) return 1;

  //OK, now do the conversion
  FileMetaData* outputMetaData{nullptr};
  TTree* outputEventTree{nullptr};
  std::set<std::string> allParents;

  for (const auto& input : inputfilenames) {
    B2INFO("processing input file: " << input);
    TFile tfile(input.c_str());
    TTree* persistent = dynamic_cast<TTree*>(tfile.Get("persistent"));
    TTree* tree = dynamic_cast<TTree*>(tfile.Get("tree"));
    FileMetaData* fileMetaData{nullptr};
    persistent->SetBranchAddress("FileMetaData", &fileMetaData);
    if (persistent->GetEntry(0) <= 0) {
      B2ERROR("Problem loading FileMetaData from " << input);
      return 1;
    }
    //first input, copy tree structure and event metadata and the whole
    //tree as it is
    if (!outputMetaData) {
      outputMetaData = new FileMetaData(*fileMetaData);
      output.cd();
      outputEventTree = tree->CloneTree(0);
    } else {
      // merge metadata here ...
      outputMetaData->setMcEvents(outputMetaData->getMcEvents() + fileMetaData->getMcEvents());
      outputMetaData->setNEvents(outputMetaData->getNEvents() + fileMetaData->getNEvents());
      // FIXME: add the rest ...

      // and set the branch addresses to copy the whole tree
      outputEventTree->CopyAddresses(tree);
    }
    for (int i = 0; i < fileMetaData->getNParents(); ++i) {
      allParents.insert(fileMetaData->getParent(i));
    }
    // Now let's copy all entries without unpacking (fast), layout the
    // baskets in an optimal order for sequential reading
    // (SortBasketByEntry) and rebuild the index in case some parts of the
    // index are missing
    outputEventTree->CopyEntries(tree, -1, "fast SortBasketsByEntry BuildIndexOnError");
    // and reset the branch addresses to not be connected anymore
    outputEventTree->CopyAddresses(tree, true);
    // finally clean up and close file.
    delete tree;
    delete persistent;
    tfile.Close();
  }
  B2INFO("Done, writing FileMetaData");
  output.cd();
  outputEventTree->Write();

  // Final changes to MetaData
  outputMetaData->setParents(std::vector<std::string>(allParents.begin(), allParents.end()));
  // FIXME: finish with remaining metadata ...

  // Create persistent tree
  TTree outputMetaDataTree("persistent", "persistent");
  outputMetaDataTree.Branch("FileMetaData", &outputMetaData);
  //FIXME: what about other stuff in persistent?
  outputMetaDataTree.Fill();
  outputMetaDataTree.Write();
  output.Close();
}
