#include <framework/modules/rootio/RootIOUtilities.h>

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
