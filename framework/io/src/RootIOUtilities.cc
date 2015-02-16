#include <framework/io/RootIOUtilities.h>

#include <framework/datastore/DataStore.h>
#include <framework/logging/Logger.h>

#include <RVersion.h>
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
#include <TCint.h>
#endif
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

std::set<std::string> RootIOUtilities::filterBranches(const std::set<std::string>& branchesToFilter, const std::vector<std::string>& branches, const std::vector<std::string>& excludeBranches, int durability)
{
  set<string> branchSet, excludeBranchSet;
  for (string b : branches) {
    if (branchesToFilter.count(b) == 0)
      B2INFO("The branch " << b << " given in " << c_SteerBranchNames[durability] << " does not exist.");
    if (!branchSet.insert(b).second)
      B2WARNING(c_SteerBranchNames[durability] << " has duplicate entry " << b);
  }
  for (string b : excludeBranches) {
    if (branchesToFilter.count(b) == 0)
      B2INFO("The branch " << b << " given in " << c_SteerExcludeBranchNames[durability] << " does not exist.");
    if (!excludeBranchSet.insert(b).second)
      B2WARNING(c_SteerExcludeBranchNames[durability] << " has duplicate entry " << b);
  }

  set<string> out, relations;
  for (string branch : branchesToFilter) {
    if (excludeBranchSet.count(branch))
      continue;
    if (branchSet.empty() or branchSet.count(branch))
      out.insert(branch);
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
  for (const string & pattern : filenames) {
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
  const Int_t major = 1000000 * experiment + run;
  const Int_t minor = event;

  if (!tree->GetTreeIndex()) {
    B2INFO("No TTreeIndex found, rebuild it...");
    buildIndex(tree);
  }
  long entry = tree->GetEntryNumberWithIndex(major, minor);

  if (entry == -1) {
    B2ERROR("Couldn't find entry with index " << major << ", " << minor);
  }
  return entry;
}

void RootIOUtilities::buildIndex(TTree* tree)
{
  tree->BuildIndex("1000000*EventMetaData.m_experiment+EventMetaData.m_run", "EventMetaData.m_event");
}

bool RootIOUtilities::hasStreamer(TClass* cl)
{
  if (cl == TObject::Class())
    return false;

  if (cl->GetClassVersion() <= 0) {
    // version number == 0 means no streamers for this class, check base classes
    TList* baseClasses = cl->GetListOfBases();
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


bool RootIOUtilities::hasCustomStreamer(TClass* cl)
{
  //does this class have a custom streamer? (magic from from TTree.cxx)
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,0,0)
  return cl->TestBit(TClass::kHasCustomStreamerMember);
#else
  return gCint->ClassInfo_RootFlag(cl->GetClassInfo()) & 1;
#endif
}

void RootIOUtilities::loadDictionaries()
{
  gSystem->Load("libdataobjects");
  gSystem->Load("libTreePlayer");
  gSystem->Load("libgenfit2");    // Because genfit2 classes need custom streamers.
  gSystem->Load("libvxd");
  gSystem->Load("libsvd");
  gSystem->Load("libpxd");
  gSystem->Load("libcdc");
}
