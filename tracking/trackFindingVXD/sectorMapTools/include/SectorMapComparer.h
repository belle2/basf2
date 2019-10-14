#pragma once

#include <TTree.h>
#include <unordered_map>
#include <TH1F.h>



/**TODOs:
  - implement cross checks for trying to compare different sectormaps, e.g. SectorSetups.
*/



namespace Belle2 {
  /**
    A root tool that compares two Sectormaps (local root files) and produces some statistics output. Yes we old people do
    use root. So get over it and open a root session to create an Object to let its magic unfold.
    This tools assumes that both sectormaps use the same setup (i.e. number of sectors on sensor, and filter configuration). It still may
    work with slightly different filters as long as the naming did not change. But if number of sectors on sensor is different the
    matching of sectors will not work and the results may become misleading!
  */
  class SectorMapComparer : public TObject {
  public:
    SectorMapComparer() {}; // = delete;
    SectorMapComparer(const std::string& SMFileFirst, const std::string& SMFileSecond) : m_SMFileName_first(SMFileFirst),
      m_SMFileName_second(SMFileSecond) {};
    ~SectorMapComparer() {}; // = default;


    /// will create lots of Canvases!!!
    void Plot();

    /// runs the comparison for all filters (and only the filters) in the sectormap files
    void Compare();


    // test function to do some quick and dirty  testing
    // TODO: remove when no testing is needed anymore
    void TEST();

  private:
    /// I was too lacy to implement a proper hash function, so I convert the three numbers into a string and use the string as hash for the map
    std::string GetHash(long l1, long l2, long l3);

    void FindTrees(TDirectory* aDir, std::vector<std::string>&  listOfTrees);

    void SetLeafAddresses(TTree* t, std::unordered_map<std::string, double>& filterVals,
                          std::unordered_map<std::string, uint>& SecIDVals);

    void FillSectorToTreeIndexMap(TTree* tree, std::unordered_map<std::string, long>& map);

    void CompareTrees(TTree* t_first, TTree* t_second);

    void ClearMaps()
    {
      m_histo_map_first.clear();
      m_histo_map_second.clear();
      m_histo_map_diff.clear();
    }

    // stores the histograms
    // WARNING: these will be reused!
    std::unordered_map<std::string, TH1F> m_histo_map_first; ///< histograms for the first sector map
    std::unordered_map<std::string, TH1F> m_histo_map_second; ///< histograms for the second sector map
    std::unordered_map<std::string, TH1F> m_histo_map_diff; ///< histograms that store differences between the two sector maps
    std::unordered_map<std::string, TH1F> m_histo_map_range; ///< histograms that store the range of that filter (max - min)
    std::unordered_map<std::string, TH1F>
    m_histo_map_unmatched; ///< histograms that store the distribution for sector combinations that are not matched between the two maps


    std::string m_SMFileName_first; ///< name of the file containing the first sectormap
    std::string m_SMFileName_second; ///< name of the file containing the second sectormap

    ClassDef(SectorMapComparer, 0)
  };
}// end namespace
