/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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

#include <queue>

using namespace Belle2;

const std::string RootIOUtilities::c_treeNames[] = { "tree", "persistent" };
const std::string RootIOUtilities::c_SteerBranchNames[] = { "branchNames", "branchNamesPersistent" };
const std::string RootIOUtilities::c_SteerExcludeBranchNames[] = { "excludeBranchNames", "excludeBranchNamesPersistent" };
const std::string RootIOUtilities::c_SteerAdditionalBranchNames[] = { "additionalBranchNames", "additionalBranchNamesPersistent" };

std::set<std::string> RootIOUtilities::filterBranches(const std::set<std::string>& branchesToFilter,
                                                      const std::vector<std::string>& branches,
                                                      const std::vector<std::string>& excludeBranches, int durability, bool quiet)
{
  std::set<std::string> branchSet, excludeBranchSet;
  for (const std::string& b : branches) {
    if (branchesToFilter.count(b) == 0 and not quiet)
      B2WARNING("The branch " << b << " given in " << c_SteerBranchNames[durability] << " does not exist.");
    if (!branchSet.insert(b).second and not quiet)
      B2WARNING(c_SteerBranchNames[durability] << " has duplicate entry " << b);
  }
  for (const std::string& b : excludeBranches) {
    // FIXME: ProcessStatistics is excluded by default but not always present. We should switch that to not write it out
    // in the first place but the info message is meaningless for almost everyone
    if (branchesToFilter.count(b) == 0 and not quiet and b != "ProcessStatistics")
      B2INFO("The branch " << b << " given in " << c_SteerExcludeBranchNames[durability] << " does not exist.");
    if (!excludeBranchSet.insert(b).second and not quiet)
      B2WARNING(c_SteerExcludeBranchNames[durability] << " has duplicate entry " << b);
  }

  std::set<std::string> out, relations, excluderelations;
  for (const std::string& branch : branchesToFilter) {
    if (excludeBranchSet.count(branch))
      continue;
    if (branchSet.empty() or branchSet.count(branch))
      out.insert(branch);
  }
  if (!excludeBranchSet.empty()) {
    //remove relations for excluded things
    for (const std::string& from : branchesToFilter) {
      for (const std::string& to : branchesToFilter) {
        std::string branch = DataStore::relationName(from, to);
        if (out.count(branch) == 0)
          continue; //not selected
        if (excludeBranchSet.count(from) == 0 and excludeBranchSet.count(to) == 0)
          continue; //at least one side should be excluded
        if (branchSet.count(branch) != 0)
          continue; //specifically included
        excluderelations.insert(branch);
      }
    }
    for (const std::string& rel : excluderelations) {
      out.erase(rel);
    }
  }
  //add relations between accepted branches
  for (const std::string& from : out) {
    for (const std::string& to : out) {
      std::string branch = DataStore::relationName(from, to);
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

size_t RootIOUtilities::setBranchStatus(TBranch* branch, bool process)
{
  size_t found{0};
  std::queue<TBranch*> branches;
  branches.emplace(branch);
  while (!branches.empty()) {
    ++found;
    auto* current = branches.front();
    branches.pop();
    // set the flag we need
    if (process) current->ResetBit(kDoNotProcess);
    else current->SetBit(kDoNotProcess);
    // add all children to the queue
    auto* children = current->GetListOfBranches();
    const auto nchildren = children->GetEntriesFast();
    for (int i = 0; i < nchildren; ++i) {
      branches.emplace(dynamic_cast<TBranch*>(children->UncheckedAt(i)));
    }
  }
  return found;
}

std::vector<std::string> RootIOUtilities::expandWordExpansions(const std::vector<std::string>& filenames)
{
  std::vector<std::string> out;
  wordexp_t expansions;
  wordexp("", &expansions, 0);
  for (const std::string& pattern : filenames) {
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
    while (auto* base = static_cast<TBaseClass*>(it())) {
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

void RootIOUtilities::setCreationData(FileMetaData& metadata)
{
  std::string site;
  char date[100];
  auto now = time(nullptr);
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
