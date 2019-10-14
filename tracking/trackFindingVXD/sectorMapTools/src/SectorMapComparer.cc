#include "tracking/trackFindingVXD/sectorMapTools/SectorMapComparer.h"

#include <TFile.h>
#include <TLeaf.h>
#include <TCanvas.h>
#include <TObjArray.h>
#include <TObject.h>

#include <vector>
#include <iostream>




using namespace Belle2;


std::string
SectorMapComparer::GetHash(long l1, long l2, long l3)
{
  std::string str = std::to_string(l1) + std::to_string(l2) + std::to_string(l3);
  return str;
}



// sets the addresses of the leafes
// assumes each leaf name is unique! (Not sure if root takes care for that)
void
SectorMapComparer::SetLeafAddresses(TTree* t, std::unordered_map<std::string, double>& filterVals,
                                    std::unordered_map<std::string, uint>& SecIDVals)
{
  TObjArray* leafList = t->GetListOfLeaves();
  for (TObject* o : *leafList) {
    TLeaf* l = (TLeaf*)o;
    std::string name = l->GetName();

    // filter sectorID leafs from the rest
    // TODO: implement cross check for leaf type. Up to now we only used TLeafD and TLeafI
    if (name.find("FullSecID") != std::string::npos) {
      SecIDVals[name] = 0;
      l->SetAddress(&(SecIDVals[name]));
    } else {
      std::cout << "name " << name << std::endl;
      filterVals[name] = 0.0;
      l->SetAddress(&(filterVals[name]));
    }

  }
}




// returns a map of sector combinations to the position in the branch
// WARNING: this messes up the addresses!!!!!!!
// TODO: fix this code! Or use the SetLeafAddresses, and remove this function completely (its only 5 lines which are relevant!)
void
SectorMapComparer::FillSectorToTreeIndexMap(TTree* tree, std::unordered_map<std::string, long>& map)
{
  // the branchnames which store the sectorIDs
  std::vector<std::string> bNames = {"innerFullSecID", "centerFullSecID", "outerFullSecID"};
  std::vector<uint> bVals = {0, 0, 0}; // lets hope there is no inner sector id 0! But need to be 0 here as later no check for existance in the map for segments, so default value is used.

  // decativate all branches to gain same speed
  for (uint i = 0; i < bNames.size(); i++) {
    TLeaf* l = tree->GetLeaf(bNames[i].c_str());
    if (l) {
      l->SetAddress(&(bVals[i]));
    }
  }

  // now loop to create the map
  for (long i = 0; i < tree->GetEntries(); i++) {
    tree->GetEntry(i);
    std::string hash = GetHash(bVals[0], bVals[1], bVals[2]);
    //std::cout << hash << " " << i << std::endl;
    map[hash] = i;
  }

}






// actually compares the trees (segment and triplet filters)
void
SectorMapComparer::CompareTrees(TTree* t_first, TTree* t_second)
{

  // clear the maps to be able to reuse them
  // TODO: acutally this is not needed as long as all trees used here have different branch names
  //ClearMaps();


  // index the second tree
  std::unordered_map<std::string, long> indexmap_t_second;
  // WARNING: this messes with leaf addresses of the sector ids
  FillSectorToTreeIndexMap(t_second, indexmap_t_second);


  std::unordered_map<std::string, double> vals_t_first;
  std::unordered_map<std::string, uint> ids_t_first;
  SetLeafAddresses(t_first, vals_t_first, ids_t_first);

  std::unordered_map<std::string, double> vals_t_second;
  std::unordered_map<std::string, uint> ids_t_second;
  SetLeafAddresses(t_second, vals_t_second, ids_t_second);


  // use the list of leaves  for indexing
  TObjArray* leafList = t_first->GetListOfLeaves();
  for (TObject* o : *leafList) {
    TLeaf* l_first = (TLeaf*)o;
    // no histograms for the sector ids
    // TODO: maybe histograms with number of friends for certain sectors are interesting es well
    TString leafName = l_first->GetName();
    if (leafName.Contains("FullSecID")) continue;

    double min = std::min(t_first->GetMinimum(leafName) , t_second->GetMinimum(leafName));
    double max = std::max(t_first->GetMaximum(leafName) , t_second->GetMaximum(leafName));
    double range = std::fabs(min - max);
    // TODO: make this configurable
    int Nbins = 100;

    // As in the current version the maps are used for several trees, cross check that histograms are not overwritten
    if (m_histo_map_first.find(leafName.Data()) != m_histo_map_first.end() ||
        m_histo_map_second.find(leafName.Data()) != m_histo_map_second.end() ||
        m_histo_map_diff.find(leafName.Data()) != m_histo_map_diff.end()) {
      //TODO: replace by B2ERROR
      std::cout << "ERROR: Histogram for key " << leafName.Data() << " already exists" << std::endl;
    }

    m_histo_map_first[ leafName.Data() ] = TH1F(leafName + "_first", leafName + "_first", Nbins, min, max);
    m_histo_map_first[ leafName.Data() ].SetDirectory(nullptr);
    m_histo_map_second[ leafName.Data() ] = TH1F(leafName + "_second", leafName + "_second", Nbins, min, max);
    m_histo_map_second[ leafName.Data() ].SetDirectory(nullptr);
    m_histo_map_diff[ leafName.Data() ] = TH1F(leafName + "_diff", leafName + "_diff", Nbins, -0.2 * range, 0.2 * range);
    m_histo_map_diff[ leafName.Data() ].SetDirectory(nullptr);
  }


  // loop over t1, and get corresponding value for t2
  for (long i = 0; i < t_first->GetEntries(); i++) {
    t_first->GetEntry(i);


    std::string hash = GetHash(ids_t_first["innerFullSecID"], ids_t_first["centerFullSecID"], ids_t_first["outerFullSecID"]);

    std::cout << hash << std::endl;
    if (indexmap_t_second.find(hash) == indexmap_t_second.end()) {
      std::cout << "sector combination not found (inner,center,outer) " << ids_t_first["innerFullSecID"] << " "
                << ids_t_first["centerFullSecID"] << " " << ids_t_first["outerFullSecID"] << std::endl;
      continue;
    } else {
      //std::cout << "found" << std::endl;
    }

    t_second->GetEntry(indexmap_t_second[hash]);

    // now fill the histograms
    for (TObject* o : *leafList) {
      TString leafName = o->GetName();
      // TODO: find a better way to destinguis the secid leaves from the others
      if (leafName.Contains("FullSecID")) continue;
      m_histo_map_first[ leafName.Data() ].Fill(vals_t_first[leafName.Data()]);
      m_histo_map_second[ leafName.Data() ].Fill(vals_t_second[leafName.Data()]);
      m_histo_map_diff[ leafName.Data() ].Fill(vals_t_first[leafName.Data()] - vals_t_second[leafName.Data()]);
    }
  }

}


// fills the listOfTrees with the names of all trees in this directory (including their full root-path)
// loops recursively over all subdirectories
void
SectorMapComparer::FindTrees(TDirectory* aDir, std::vector<std::string>&  listOfTrees)
{
  TList* keys = aDir->GetListOfKeys();
  for (TObject* akey : *keys) {

    // there should be no check needed as each key should be attached to an object
    TObject* o = aDir->Get(akey->GetName());

    //std::cout << o->ClassName() << std::endl;

    if (o->InheritsFrom(TDirectory::Class())) FindTrees((TDirectory*)o, listOfTrees);
    if (o->InheritsFrom(TTree::Class())) {
      listOfTrees.emplace_back(std::string(aDir->GetPath()) + std::string("/") + std::string(o->GetName()));
    }
  }

}


void
SectorMapComparer::Plot()
{
  for (const auto& hist : m_histo_map_first) {
    std::string aName = hist.first;

    TCanvas* can = new TCanvas((aName + "_can").c_str(), aName.c_str());
    // dont want it yet to be put into some open files
    // can->SetDirectory(nullptr);
    can->Divide(1, 2);
    can->cd(1);
    m_histo_map_first[ aName ].DrawCopy();
    m_histo_map_second[ aName ].SetLineColor(kRed);
    m_histo_map_second[ aName ].DrawCopy("same");

    can->cd(2);
    m_histo_map_diff[ aName ].DrawCopy();

  }

}


void
SectorMapComparer::Compare()
{
  std::cout << "comparing SectorMaps in the following files" << std::endl;
  std::cout << "first:  " << m_SMFileName_first << std::endl;
  std::cout << "second: " << m_SMFileName_second << std::endl;

  TFile* f_first = TFile::Open(m_SMFileName_first.c_str());
  TFile* f_second = TFile::Open(m_SMFileName_second.c_str());

  if (!f_first->IsOpen() || !f_first->IsOpen()) {
    // TODO: replace by error
    std::cout << "ERROR: one of the files not open" << std::endl;

    if (f_first) f_first->Close();
    if (f_second) f_second->Close();
    return;
  }

  // get the list of trees from the first file (for second file assume same trees)
  std::vector<std::string> listOfTrees;
  FindTrees(f_first, listOfTrees);

  for (const std::string& tname_first : listOfTrees) {
    // may need to apply some filtering to the trees, e.g. Setups

    std::string tname_second = tname_first;
    tname_second.replace(0, m_SMFileName_first.length(), m_SMFileName_second);




    TTree* t_first = (TTree*)f_first->Get(tname_first.c_str());
    TTree* t_second = (TTree*)f_second->Get(tname_second.c_str());

    std::cout << tname_first << " " << t_first <<  std::endl;
    std::cout << tname_second << " " << t_second << std::endl;

    if (!t_first || !t_second) {
      std::cout << "ERROR: one of the trees not found" << std::endl;
      continue;
    }

    CompareTrees(t_first, t_second);


  }

  f_first->Close();
  f_second->Close();
}



void
SectorMapComparer::TEST()
{

  TFile* f1 = TFile::Open("/home/thomas/belle2/releases/myHead/buff/SectorMaps.root");
  //TTree * t1 = (TTree*)f1.Get("SVDOnlyDefault/TripletsFilters");
  TTree* t1 = (TTree*)f1->Get("SVDOnlyDefault/SegmentFilters");

  TFile* f2 = TFile::Open("/home/thomas/belle2/releases/myHead/buff/dbstore_SVDSectorMap_v000.root_rev_18.root");
  //TTree * t2 = (TTree*)f2.Get("SVDOnlyDefault/TripletsFilters");
  TTree* t2 = (TTree*)f2->Get("SVDOnlyDefault/SegmentFilters");



  CompareTrees(t1, t2);

}






