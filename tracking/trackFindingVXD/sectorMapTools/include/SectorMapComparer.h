#pragma once

#include <framework/logging/Logger.h>

#include <TTree.h>
#include <unordered_map>
#include <TH1F.h>


namespace Belle2 {
  /**
    A root tool that compares two Sectormaps (local root files) and produces some statistics output. Yes we old people do
    use root. So get over it and open a root session to create an Object to let its magic unfold.
    This tools assumes that both sectormaps use the same setup (i.e. number of sectors on sensor, and filter configuration). It still may
    work with slightly different filters as long as the naming did not change. But if the number of sectors on sensor is different the
    matching of sectors will not work and the results may become misleading!

    Per default only sector combinations which are matched between both sectormaps are drawn.
    If the option unmatchedEntries==true is set when calling CompareMaps only the sector combinations which are not matched from the first sector map are plotted.
  */
  class SectorMapComparer : public TObject {
  public:
    SectorMapComparer() = delete;
    SectorMapComparer(const std::string& SMFileFirst, const std::string& SMFileSecond);

    ~SectorMapComparer() {}; // = default;


    /// will create lots of Canvases!!!
    void Plot(bool logScale = true, TString pdfFileName = "");

    /// runs the comparison for all filters (and only the filters) in the sectormap files
    void CompareMaps(TString setup, bool unmatchedEntries = false);

    /// lists all setups of sectormaps included in the two files
    void ShowSetups()
    {
      B2INFO(std::endl);
      ShowSetups(m_SMFileName_first);
      B2INFO(std::endl);
      ShowSetups(m_SMFileName_second);
    }
    void ShowSetups(TString sectorMapFileName);

    void ShowFiles()
    {
      B2INFO("Following files will be compared:");
      B2INFO("=================================");
      B2INFO("First : " << m_SMFileName_first);
      B2INFO("Second: " << m_SMFileName_second);
    }

  private:
    /// I was too lacy to implement a proper hash function, so I convert the three numbers into a string and use the string as hash for the map
    std::string GetHash(long l1, long l2, long l3);

    void FindTrees(TDirectory* aDir, std::vector<std::string>&  listOfTrees);

    void SetLeafAddresses(TTree* t, std::unordered_map<std::string, double>& filterVals,
                          std::unordered_map<std::string, uint>& SecIDVals);

    void FillSectorToTreeIndexMap(TTree* tree, std::unordered_map<std::string, long>& map);

    void CompareTrees(TTree* t_first, TTree* t_second, bool unmatchedEntries = false);

    void ClearMaps()
    {
      m_histo_map_first.clear();
      m_histo_map_second.clear();
      m_histo_map_diff.clear();
      m_histo_map_range_first.clear();
      m_histo_map_range_second.clear();
    }

    // stores the histograms
    // WARNING: these will be reused!
    std::unordered_map<std::string, TH1F> m_histo_map_first; ///< histograms for the first sector map
    std::unordered_map<std::string, TH1F> m_histo_map_second; ///< histograms for the second sector map
    std::unordered_map<std::string, TH1F> m_histo_map_diff; ///< histograms that store differences between the two sector maps
    std::unordered_map<std::string, TH1F> m_histo_map_range_first; ///< histograms that store the range of that filter (max - min)
    std::unordered_map<std::string, TH1F> m_histo_map_range_second; ///< histograms that store the range of that filter (max - min)


    std::string m_SMFileName_first; ///< name of the file containing the first sectormap
    std::string m_SMFileName_second; ///< name of the file containing the second sectormap

    std::string m_setupsTreeName = "Setups"; ///< name of the tree all the setups are stored
    std::string m_setupsBranchName = "name"; ///< name of the branch in the setups tree that holds the names of the setups

    // we dont want to write out this class, so version number should stay 0
    ClassDef(SectorMapComparer, 0)
  };
}// end namespace
