#include <framework/io/RootIOUtilities.h>

#include <framework/datastore/DataStore.h>
#include <framework/dataobjects/FileMetaData.h>
#include <framework/logging/Logger.h>

#include <RVersion.h>
#include <TTree.h>
#include <TList.h>
#include <TClass.h>
#include <TBaseClass.h>
#include <TSystem.h>

#include <wordexp.h>

#include <algorithm>


using namespace std;
using namespace Belle2;

const std::string RootIOUtilities::c_treeNames[] = { "tree", "persistent" };
const std::string RootIOUtilities::c_SteerBranchNames[] = { "branchNames", "branchNamesPersistent" };
const std::string RootIOUtilities::c_SteerExcludeBranchNames[] = { "excludeBranchNames", "excludeBranchNamesPersistent" };
const std::string RootIOUtilities::c_SteerAdditionalBranchNames[] = { "additionalBranchNames", "additionalBranchNamesPersistent" };

std::set<std::string> RootIOUtilities::filterBranches(const std::set<std::string>& branchesToFilter,
                                                      const std::vector<std::string>& branches, const std::vector<std::string>& excludeBranches, int durability)
{
  set<string> branchSet, excludeBranchSet;
  for (string b : branches) {
    if (branchesToFilter.count(b) == 0)
      B2WARNING("The branch " << b << " given in " << c_SteerBranchNames[durability] << " does not exist.");
    if (!branchSet.insert(b).second)
      B2WARNING(c_SteerBranchNames[durability] << " has duplicate entry " << b);
  }
  for (string b : excludeBranches) {
    if (branchesToFilter.count(b) == 0)
      B2INFO("The branch " << b << " given in " << c_SteerExcludeBranchNames[durability] << " does not exist.");
    if (!excludeBranchSet.insert(b).second)
      B2WARNING(c_SteerExcludeBranchNames[durability] << " has duplicate entry " << b);
  }

  set<string> out, relations, excluderelations;
  for (string branch : branchesToFilter) {
    if (excludeBranchSet.count(branch))
      continue;
    if (branchSet.empty() or branchSet.count(branch))
      out.insert(branch);
  }
  if (!excludeBranchSet.empty()) {
    //remove relations for excluded things
    for (string from : branchesToFilter) {
      for (string to : branchesToFilter) {
        string branch = DataStore::relationName(from, to);
        if (out.count(branch) == 0)
          continue; //not selected
        if (excludeBranchSet.count(from) == 0 and excludeBranchSet.count(to) == 0)
          continue; //at least one side should be excluded
        if (branchSet.count(branch) != 0)
          continue; //specifically included
        excluderelations.insert(branch);
      }
    }
    for (string rel : excluderelations) {
      out.erase(rel);
    }
  }
  //add relations between accepted branches
  for (string from : out) {
    for (string to : out) {
      string branch = DataStore::relationName(from, to);
      if (branchesToFilter.count(branch) == 0)
        continue; //not in input
      if (excludeBranchSet.count(branch))
        continue;
      relations.insert(branch);
    }
  }
  out.insert(relations.begin(), relations.end());
  return out;
}

std::vector<std::string> RootIOUtilities::expandWordExpansions(const std::vector<std::string>& filenames)
{
  vector<string> out;
  wordexp_t expansions;
  wordexp("", &expansions, 0);
  for (const string& pattern : filenames) {
    if (wordexp(pattern.c_str(), &expansions, WRDE_APPEND | WRDE_NOCMD | WRDE_UNDEF) != 0) {
      B2ERROR("Failed to expand pattern '" << pattern << "'!");
    }
  }
  out.resize(expansions.we_wordc);
  for (unsigned int i = 0; i < expansions.we_wordc; i++) {
    out[i] = expansions.we_wordv[i];
  }
  wordfree(&expansions);
  return out;
}


long RootIOUtilities::getEntryNumberWithEvtRunExp(TTree* tree, long event, long run, long experiment)
{
  const long major = 1000000 * experiment + run;
  const long minor = event;

  if (!tree->GetTreeIndex()) {
    B2DEBUG(100, "No TTreeIndex found, rebuild it...");
    buildIndex(tree);
  }
  long entry = tree->GetEntryNumberWithIndex(major, minor);

  if (entry == -1) {
    //should be handled by caller
    B2DEBUG(100, "Couldn't find entry (" << event << ", " << run << ", " << experiment << ") in file! (major index: " << major <<
            ", minor index: " << minor << ")");
  }
  return entry;
}

void RootIOUtilities::buildIndex(TTree* tree)
{
  tree->BuildIndex("1000000*EventMetaData.m_experiment+EventMetaData.m_run", "EventMetaData.m_event");
}

bool RootIOUtilities::hasStreamer(const TClass* cl)
{
  if (cl == TObject::Class())
    return false;

  if (cl->GetClassVersion() <= 0) {
    // version number == 0 means no streamers for this class, check base classes
    TList* baseClasses = const_cast<TClass*>(cl)->GetListOfBases(); //method might update an internal cache, but is const otherwise
    TIter it(baseClasses);
    while (TBaseClass* base = static_cast<TBaseClass*>(it())) {
      if (hasStreamer(base->GetClassPointer()))
        return true;
    }
    //nothing found
    return false;
  } else {
    return true;
  }
}


bool RootIOUtilities::hasCustomStreamer(const TClass* cl)
{
  //does this class have a custom streamer? (magic from from TTree.cxx)
  return cl->TestBit(TClass::kHasCustomStreamerMember);
}

void RootIOUtilities::loadDictionaries()
{
  static bool loaded = false;
  if (loaded)
    return;

  gSystem->Load("libdataobjects");
  gSystem->Load("libTreePlayer");
  gSystem->Load("libgenfit2");    // Because genfit2 classes need custom streamers.
  gSystem->Load("libvxd");
  gSystem->Load("libsvd");
  gSystem->Load("libpxd");
  gSystem->Load("libcdc");
  loaded = true;
}

void RootIOUtilities::setCreationData(FileMetaData& metadata)
{
  std::string site;
  char date[100];
  auto now = time(0);
  strftime(date, 100, "%Y-%m-%d %H:%M:%S", gmtime(&now));
  const char* belle2_site = getenv("BELLE2_SITE");
  if (belle2_site) {
    site = belle2_site;
  } else {
    char hostname[1024];
    gethostname(hostname, 1023); //will not work well for ipv6
    hostname[1023] = '\0'; //if result is truncated, terminating null byte may be missing
    site = hostname;
  }
  const char* user = getenv("BELLE2_USER");
  if (!user) user = getenv("USER");
  if (!user) user = getlogin();
  if (!user) user = "unknown";
  auto commitid = RootIOUtilities::getCommitID();
  metadata.setCreationData(date, site, user, commitid);
}

std::string RootIOUtilities::getCommitID()
{
  return GIT_COMMITID;
}
