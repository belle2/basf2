#include <framework/modules/rootio/RootIOUtilities.h>

#include <framework/logging/Logger.h>

#include <TTree.h>

#include <algorithm>


using namespace Belle2;

const std::string RootIOUtilities::c_treeNames[] = { "tree", "persistent" };
const std::string RootIOUtilities::c_SteerBranchNames[] = { "branchNames", "branchNamesPersistent" };
const std::string RootIOUtilities::c_SteerExcludeBranchNames[] = { "excludeBranchNames", "excludeBranchNamesPersistent" };

bool RootIOUtilities::makeBranchNamesUnique(std::vector<std::string>& stringlist)
{
  const size_t oldsize = stringlist.size();
  sort(stringlist.begin(), stringlist.end());
  stringlist.resize(unique(stringlist.begin(), stringlist.end()) - stringlist.begin());

  return (oldsize != stringlist.size());
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
