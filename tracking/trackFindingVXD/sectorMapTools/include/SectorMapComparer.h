/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Lueck                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#pragma once

#include <framework/logging/Logger.h>

#include <TTree.h>
#include <TH1F.h>

#include <unordered_map>
#include <iostream>

namespace Belle2 {
  /**
    A root tool that compares two Sectormaps (local root files) and produces some statistics output. Yes we old people do
    use root. So get over it and open a root session to create an Object to let its magic unfold.
    This tool assumes that both sectormaps use the same setup (i.e. number of sectors on sensor, and filter configuration). It still may
    work with slightly different filters as long as the naming did not change. But if the number of sectors on sensor is different the
    matching of sectors will not work and the results may become misleading!

    The output will be several plots comparing the filter cuts (min, max) for each of the contained filters. In
    addition a simple counting of sector connections is provided.

    Per default only sector combinations which are matched between both sectormaps are drawn.
    If the option unmatchedEntries==true is set when calling CompareMaps only the sector combinations which are not matched from the first sector map are plotted.
  */
  class SectorMapComparer : public TObject {
  public:
    /// dont use this constructor
    SectorMapComparer() = delete;
    /**
      constructor
       @param SMFileFirst: root file name the first sector map is contained in
       @param SMFileSecond: root file name the second sector map is contained in
    */
    SectorMapComparer(const std::string& SMFileFirst, const std::string& SMFileSecond);

    /// default destructor
    ~SectorMapComparer() = default;


    /** will create lots of Canvases!!!
      @param logScale: if true the y-axis will be log scale
      @param pdfFileName: name of the output file, has to be a file format which is supported by TCanvas::SaveAs (e.g. .pdf)
        if empty "" no output file will be created
      @param drawLegend: if true a legend is drawn, may hide interisting parts so one can deactivate it
    */
    void plot(bool logScale = true, TString pdfFileName = "", bool drawLegend = true);

    /** Will create canvases showing statistics
      @param logScale: if true the y-axis will be log scale
      @param pdfFileName: name of the output file, has to a file format which is supported by TCanvas::SaveAs (e.g. .pdf)
        if empty "" no output file will be created
      @param drawLegend: if true a legend is drawn, may hide interisting parts so one can deactivate it
    */
    void plotSectorStats(bool logScale = true, TString pdfFileName = "", bool drawLegend = true);

    /// runs the comparison for all filters (and only the filters) in the sectormap files
    void compareMaps(TString setup, bool unmatchedEntries = false);

    /// lists all setups of sectormaps included in the two files
    void showSetups()
    {
      std::cout << std::endl;
      showSetups(m_SMFileName_first);
      std::cout << std::endl;
      showSetups(m_SMFileName_second);
    }

    /// lists all sector map setups in the specified file
    void showSetups(TString sectorMapFileName);

    /// lists the names of the files which are used
    void showFiles()
    {
      B2INFO("Following files will be compared:");
      B2INFO("=================================");
      B2INFO("First : " << m_SMFileName_first);
      B2INFO("Second: " << m_SMFileName_second);
    }


  private:


    /** counts all connections for given layer, ladder, sensor, sector. If -1 is given for any of those it is summed over the corresponding entries
     This has to be called after the CompareMaps function, else the result will be zero
      @param map: reference to map mapping from fullsecid to number of times
      @param layer, ladder, sensor, sector: layer, ladder, sensor, sector number, if -1 it will be summed over
    */
    uint countConnections(const std::unordered_map<uint, uint>& map,  int layer = -1, int ladder = -1, int sensor = -1,
                          int sector = -1);

    /** I was too lacy to implement a proper hash function, so I convert the three numbers into a string and use the string as hash for the map
      @param l1, l2, l3: just three numbers of type long
      @return: a string which combines the three numbers which can be used as hash as the hash function for std::string already exists
    */
    std::string getHash(long l1, long l2, long l3);

    /** helper function that returns a list of names for all trees contained in the TDirectory including sub-directories
      @param aDir: a root directory pointer (e.g. TFile *)
      @param listOfTrees: this vector will be filled with the names of the tree (NOTE: the name contains the full path (including file name))
    */
    void findTrees(TDirectory* aDir, std::vector<std::string>&  listOfTrees);

    /** automatically sets the TLeaf addresses for a tree. Currently only TLeafI and TLeafD are supported
        @param t: the tree for setting the addresses
        @param filterVals: the targets for the TLeafD addresses, will be filled in the function
        @param SecIDVals: the targets for the TLeafI addresses, will be filled in the function
    */
    void setLeafAddresses(TTree* t, std::unordered_map<std::string, double>& filterVals,
                          std::unordered_map<std::string, uint>& SecIDVals);

    /**
      fills a map that maps sector combinations in the tree to string which is a hash combining the three (two) sector ids
      @param tree: the tree to be mapped
      @param map: the map which will be filled in the function
    */
    void fillSectorToTreeIndexMap(TTree* tree, std::unordered_map<std::string, long>& map);

    /** makes the comparison of two trees, and fills histograms and certain maps
    @param t_first: first tree
    @param t_second: secdon tree
    @param unmatchedEntries: if true only the unmatched entries will be filled (only for the first tree)
    */
    void compareTrees(TTree* t_first, TTree* t_second, bool unmatchedEntries = false);

    /** helper that clears all the maps used*/
    void clearMaps()
    {
      m_histo_map_first.clear();
      m_histo_map_second.clear();
      m_histo_map_diff.clear();
      m_histo_map_range_first.clear();
      m_histo_map_range_second.clear();
      m_map_N2HitCombs.clear();
      m_map_N2HitCombs_matched.clear();
      m_map_N3HitCombs.clear();
      m_map_N3HitCombs_matched.clear();
    }

    // stores the histograms
    // WARNING: these will be reused!
    std::unordered_map<std::string, TH1F> m_histo_map_first; ///< histograms for the first sector map
    std::unordered_map<std::string, TH1F> m_histo_map_second; ///< histograms for the second sector map
    std::unordered_map<std::string, TH1F> m_histo_map_diff; ///< histograms that store differences between the two sector maps
    std::unordered_map<std::string, TH1F> m_histo_map_range_first; ///< histograms that store the range of that filter (max - min)
    std::unordered_map<std::string, TH1F> m_histo_map_range_second; ///< histograms that store the range of that filter (max - min)

    /// count the number of 2Hit connections for each sector,
    std::unordered_map<uint, uint> m_map_N2HitCombs;
    /// count the number of 2Hit connections for each sector, only if sector combination was "matched" (Note: meaning of "matched" depends on the setting)
    std::unordered_map<uint, uint> m_map_N2HitCombs_matched;

    /// count the number of 3Hit connections for each sector, can be used to do some statistics
    std::unordered_map<uint, uint> m_map_N3HitCombs;
    /// count the number of 3Hit connections for each sector, only if sector combination was "matched" (Note: meaning of "matched" depends on the setting)
    std::unordered_map<uint, uint> m_map_N3HitCombs_matched;



    std::string m_SMFileName_first; ///< name of the file containing the first sectormap
    std::string m_SMFileName_second; ///< name of the file containing the second sectormap

    std::string m_setupsTreeName = "Setups"; ///< name of the tree all the setups are stored
    std::string m_setupsBranchName = "name"; ///< name of the branch in the setups tree that holds the names of the setups

    /// remember if CompareMaps already has been run, to give warnings if functions are called that need CompareMaps to have run
    bool m_isCompared = false;

    // we dont want to write out this class, so version number should stay 0
    ClassDef(SectorMapComparer, 0)
  };
}// end namespace
