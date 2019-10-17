#include "tracking/trackFindingVXD/sectorMapTools/SectorMapComparer.h"

#include <framework/logging/LogConfig.h>

#include <TFile.h>
#include <TLeaf.h>
#include <TLeafI.h>
#include <TLeafD.h>
#include <TCanvas.h>
#include <TObjArray.h>
#include <TObject.h>

#include <vector>
#include <iostream>




using namespace Belle2;

SectorMapComparer::SectorMapComparer(const std::string& SMFileFirst,
                                     const std::string& SMFileSecond) : m_SMFileName_first(SMFileFirst),
  m_SMFileName_second(SMFileSecond)
{
  Belle2::LogSystem::Instance().getLogConfig()->setLogLevel(LogConfig::c_Info);
};


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
    // cannot use TClass pointer here as it is TLeaf and not TLeafD or TLeafI, ClassName is virtual (gives the name of the derived)
    TString classname = l->ClassName();

    // filter leafs of type TLeafI (sector IDs) from the
    if (classname.EqualTo("TLeafI")) {
      SecIDVals[name] = 0;
      l->SetAddress(&(SecIDVals[name]));
    } else if (classname.EqualTo("TLeafD")) {
      filterVals[name] = 0.0;
      l->SetAddress(&(filterVals[name]));
    } else {
      B2WARNING("Unsupported TLeaf type: " << l->ClassName() << ". Will skip this TLeaf.");
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
SectorMapComparer::CompareTrees(TTree* t_first, TTree* t_second, bool unmatchedEntries)
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

  // some cross checks
  if (vals_t_second.size() != vals_t_first.size()) {
    B2WARNING("Number of filters stored in the two SectorMaps seem to differ! This is per se not dangerous, but some cuts may be compared to zero.");
  }

  // Creating histograms, using the list of leaves  for indexing the histograms
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

      B2ERROR("ERROR: Histogram for key " << leafName.Data() << " already exists");
    }


    m_histo_map_first[ leafName.Data() ] = TH1F(leafName + "_first", leafName, Nbins, min, max);
    m_histo_map_first[ leafName.Data() ].SetDirectory(nullptr);
    m_histo_map_second[ leafName.Data() ] = TH1F(leafName + "_second", leafName , Nbins, min, max);
    m_histo_map_second[ leafName.Data() ].SetDirectory(nullptr);
    m_histo_map_diff[ leafName.Data() ] = TH1F(leafName + "_diff", "difference " + leafName + " (SM1 - SM2)", Nbins, -0.2 * range,
                                               0.2 * range);
    m_histo_map_diff[ leafName.Data() ].SetDirectory(nullptr);

    // histograms containing the max minus the min
    TString niceName = leafName;
    niceName.ReplaceAll("_min", "");
    niceName.ReplaceAll("_max", "");
    m_histo_map_range_first[ leafName.Data() ] = TH1F(leafName + "_range_first", niceName + " range (max - min)", Nbins, 0, range);
    m_histo_map_range_first[ leafName.Data() ].SetDirectory(nullptr);
    m_histo_map_range_second[ leafName.Data() ] = TH1F(leafName + "_range_second", niceName + " range (max - min)", Nbins, 0, range);
    m_histo_map_range_second[ leafName.Data() ].SetDirectory(nullptr);
  }


  // loop over t1, and get corresponding value for t2
  for (long i = 0; i < t_first->GetEntries(); i++) {
    t_first->GetEntry(i);

    std::string hash = GetHash(ids_t_first["innerFullSecID"], ids_t_first["centerFullSecID"], ids_t_first["outerFullSecID"]);

    // filter connections inside both sector maps, if unmatchedEntries is true only the unmatched entries will be selected
    if (indexmap_t_second.find(hash) == indexmap_t_second.end()) {
      // case this sector combination not matched, can be thrown away if  unmatchedEntries is set false
      if (!unmatchedEntries) continue;
    } else {
      // sector combination is matched, read second tree and skip if unmatchedEntries is set
      t_second->GetEntry(indexmap_t_second[hash]);
      if (unmatchedEntries) continue;
    }


    // now fill the histograms
    // NOTE: in case unmatchedEntries==true the values in the the second tree do not make any sense, so not fill any histograms with them
    for (TObject* o : *leafList) {
      TString leafName = o->GetName();
      // TODO: find a better way to destinguis the secid leaves from the others
      if (leafName.Contains("FullSecID")) continue;
      m_histo_map_first[ leafName.Data() ].Fill(vals_t_first[leafName.Data()]);
      if (!unmatchedEntries) m_histo_map_second[ leafName.Data() ].Fill(vals_t_second[leafName.Data()]);
      if (!unmatchedEntries) m_histo_map_diff[ leafName.Data() ].Fill(vals_t_first[leafName.Data()] - vals_t_second[leafName.Data()]);
      // calculate the range only for the max
      if (leafName.Contains("_max")) {
        TString minLeafName = leafName;
        minLeafName.ReplaceAll("_max", "_min");
        m_histo_map_range_first[ leafName.Data() ].Fill(vals_t_first[leafName.Data()] - vals_t_first[minLeafName.Data()]);
        if (!unmatchedEntries) m_histo_map_range_second[ leafName.Data() ].Fill(vals_t_second[leafName.Data()] -
              vals_t_second[minLeafName.Data()]);
      }
    }
  }

}

void
SectorMapComparer::ShowSetups(TString secmapFileName)
{
  TFile* f = TFile::Open(secmapFileName);
  if (!f->IsOpen()) {
    B2WARNING("File not opened: " << secmapFileName);
    return;
  }

  TTree* t = (TTree*)f->Get(m_setupsTreeName.c_str());
  if (t == nullptr) {
    B2WARNING("tree not found! tree name: " << m_setupsTreeName);
    return;
  }

  TString* setupKeyName = nullptr;
  t->SetBranchAddress(m_setupsBranchName.c_str(), & setupKeyName);
  if (setupKeyName == nullptr) {
    B2WARNING("setupKeyName not found");
    return;
  }

  B2INFO("Following setups found for file: " << secmapFileName);
  B2INFO("================================ ");
  for (long i = 0; i < t->GetEntries(); ++i) {
    t->GetEntry();
    B2INFO(*setupKeyName);
  }

  if (setupKeyName) delete setupKeyName;
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
SectorMapComparer::Plot(bool logScale, TString pdfFileName)
{
  int n = m_histo_map_first.size();
  int counter = 1;
  for (const auto& hist : m_histo_map_first) {
    std::string aName = hist.first;
    TCanvas* can = new TCanvas((aName + "_can").c_str(), aName.c_str());
    can->Divide(1, 3);

    can->cd(1)->SetLogy((int)logScale);
    m_histo_map_first[ aName ].DrawCopy();
    m_histo_map_second[ aName ].SetLineColor(kRed);
    m_histo_map_second[ aName ].DrawCopy("same");

    can->cd(2)->SetLogy((int)logScale);;
    m_histo_map_diff[ aName ].DrawCopy();

    if (TString(aName).Contains("_max")) {
      can->cd(3)->SetLogy((int)logScale);
      m_histo_map_range_first[ aName ].DrawCopy();
      m_histo_map_range_second[ aName ].SetLineColor(kRed);
      m_histo_map_range_second[ aName ].DrawCopy("same");
    }

    // if file name is specified plot directly to pdf file
    if (pdfFileName != "") {
      if (n == 1) can->SaveAs(pdfFileName);
      else {
        if (counter == 1) can->SaveAs(pdfFileName + "(");
        else if (counter == n) can->SaveAs(pdfFileName + ")");
        else can->SaveAs(pdfFileName);
      }
    }
    counter++;
  }

}


void
SectorMapComparer::CompareMaps(TString setupName, bool unmatchedEntries)
{
  B2INFO("comparing SectorMaps in the following files");
  B2INFO("first:  " << m_SMFileName_first);
  B2INFO("second: " << m_SMFileName_second);

  TFile* f_first = TFile::Open(m_SMFileName_first.c_str());
  TFile* f_second = TFile::Open(m_SMFileName_second.c_str());

  if (!f_first->IsOpen() || !f_first->IsOpen()) {
    B2ERROR("ERROR: one of the files not open");

    if (f_first) f_first->Close();
    if (f_second) f_second->Close();
    return;
  }

  // get the list of trees from the first file (for second file assume same trees)
  std::vector<std::string> listOfTrees;
  FindTrees(f_first, listOfTrees);

  // clear the maps, else old results will be mixed with this run
  ClearMaps();

  for (const std::string& tname_first : listOfTrees) {

    // only look at the trees for the specified Setup
    if (!TString(tname_first).Contains(setupName)) continue;


    // assume second file has
    std::string tname_second = tname_first;
    tname_second.replace(0, m_SMFileName_first.length(), m_SMFileName_second);



    TTree* t_first = (TTree*)f_first->Get(tname_first.c_str());
    TTree* t_second = (TTree*)f_second->Get(tname_second.c_str());

    // filter out the sector id and test for consistency
    if (TString(tname_first).Contains("CompactSecIDs")) {
      if (t_first->GetEntries() != t_second->GetEntries())
        B2FATAL("The number of entries in CompactSecIDs should be identical! This indicates that two sectormaps with"
                " different sectors on sensors are compared which will lead to wrong results! " << std::endl <<
                "# CompactSecIDs SM1: " << t_first->GetEntries() << std::endl <<
                "# CompactSecIDs SM2: " << t_second->GetEntries() << std::endl
               );
      // we dont compare those
      continue;
    }


    B2INFO("Comparing tree: " << tname_first << " with tree: " << tname_second <<  std::endl);

    if (!t_first || !t_second) {
      B2ERROR("One of the trees not found!");
      continue;
    }


    CompareTrees(t_first, t_second, unmatchedEntries);


  }

  f_first->Close();
  f_second->Close();
}








