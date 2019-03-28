/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marco Milesi                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

// FRAMEWORK
#include <framework/gearbox/Const.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>
#include <framework/database/DBImportArray.h>

// MVA
#include <mva/interface/Weightfile.h>

// ROOT
#include <TObject.h>
#include <TH2F.h>

//BOOST
#include <boost/algorithm/string/predicate.hpp>
#include <boost/property_tree/xml_parser.hpp>

//C++
#include <unordered_map>


namespace Belle2 {

  /**
   * Class to contain the payload of MVA weightfiles needed for charged particle identification.
   */
  class ChargedPidMVAWeights : public TObject {

    typedef std::unordered_map<int, TH2F*> ClusterThetaPGridByParticle; /**< Typedef */
    typedef std::unordered_map<int, std::vector<std::string> > WeightfilesByParticle; /**< Typedef */

  public:

    /**
     * Default constructor, necessary for ROOT to stream the object.
     */
    ChargedPidMVAWeights() {}

    /**
     * Destructor.
     */
    ~ChargedPidMVAWeights() {};

    /**
     * Set the (cluster theta, p) grid of bins for this particle mass hypothesis' pdgId into the payload.
     * @param pdg the particle mass hypothesis' pdgId.
     * @param grid the 2D histogram w/ the bin grid.
    */
    void storeClusterThetaPGrid(const int pdg, TH2F* grid)
    {
      if (!isValidPdg(pdg)) { B2FATAL("PDG: " << pdg << " is not that of a valid charged particle! Aborting..."); }
      m_grids[pdg] = grid;
    }


    /**
     * Store the list of MVA weights for this particle mass hypothesis' pdgId into the payload.
     * NB: the caller MUST ensure that the order in the list of file paths corresponds to the
     * one of the linearised TH2F grid object, set in the payload via storeClusterThetaPGrid()
     * @param pdg the particle mass hypothesis' pdgId.
     * @param filepaths a list of xml (root) file paths for several (clusterTheta,p) bins.
     */
    void storeMVAWeights(const int pdg, const std::vector<std::string>& filepaths)
    {

      if (!isValidPdg(pdg)) { B2FATAL("PDG: " << pdg << " is not that of a valid charged particle! Aborting..."); }

      for (const auto& path : filepaths) {

        Belle2::MVA::Weightfile weightfile;
        if (boost::ends_with(path, ".root")) {
          weightfile = Belle2::MVA::Weightfile::loadFromROOTFile(path);
        } else  if (boost::ends_with(path, ".xml")) {
          weightfile = Belle2::MVA::Weightfile::loadFromXMLFile(path);
        } else {
          B2WARNING("Unkown file extension for file: " << path << ", fallback to xml...");
          weightfile = Belle2::MVA::Weightfile::loadFromXMLFile(path);
        }

        // Serialize the MVA::Weightfile object into a string for storage in the database,
        // otherwise there are issues w/ dictionary generation for the payload class...
        std::stringstream ss;
        Belle2::MVA::Weightfile::saveToStream(weightfile, ss);
        m_weightfiles[pdg].push_back(ss.str());

      }

    }

    /**
     * Get the list of (serialized) MVA weightfiles stored in the payload for a given pdgId.
     * @param pdg the particle mass hypothesis' pdgId.
     */
    const std::vector<std::string>* getMVAWeights(const int pdg) const
    {
      return &(m_weightfiles.at(pdg));
    }

    /**
     * Get the index of the weightfile of interest, for a given particle mass hypothesis, clusterTheta, and p.
     * The same index can be used to look up the correct MVA Expert and Dataset in the application module,
     * hence we believe it's more useful to return the index rather than a pointer to the weightfile itself.
     * @param part the particle mass hypothesis.
     * @param theta the particle polar angle (from the ECL cluster) in [rad].
     * @param p the particle momentum (from the track) in [GeV/c].
     * @return the index of the weightfile of interest from the array of weightfiles.
    */
    unsigned int getMVAWeightIdx(const Const::ChargedStable& part, const double& theta, const double& p) const
    {

      const TH2F* grid = m_grids.at(part.getPDGCode());

      int nbinsx = grid->GetXaxis()->GetNbins(); // nr. of theta (visible) bins, along X.

      int glob_bin_idx = findBin(grid, theta, p);
      int j, i, k;
      grid->GetBinXYZ(glob_bin_idx, j, i, k);

      // The index of the linearised 2D (theta,p) grid.
      // The unit offset is b/c ROOT sets global bin idx also for overflows and underflows.
      return (j - 1) + nbinsx * (i - 1);

    }

  private:

    /**
     * Find global bin index of a 2D histogram for the given (x, y) values.
     * This method had to be re-implemented b/c ROOT has no const version of TH1::FindBin() :(
     * @param x value along the x axis.
     * @param y value along the y axis.
     * @return the global linearised bin index.
    */
    int findBin(const TH2F* hist, const double& x, const double& y) const
    {

      int nbinsx_vis = hist->GetXaxis()->GetNbins();
      int nbinsy_vis = hist->GetYaxis()->GetNbins();

      double xx = x;
      double yy = y;

      // If x, y are outside of the 2D histogram grid (visible) range, set their value to
      // fall in the last (first) bin before (after) overflow (underflow).
      if (x < hist->GetXaxis()->GetBinLowEdge(1))               { xx = hist->GetXaxis()->GetBinCenter(1); }
      if (x >= hist->GetXaxis()->GetBinLowEdge(nbinsx_vis + 1)) { xx = hist->GetXaxis()->GetBinCenter(nbinsx_vis); }
      if (y < hist->GetYaxis()->GetBinLowEdge(1))               { yy = hist->GetYaxis()->GetBinCenter(1); }
      if (y >= hist->GetYaxis()->GetBinLowEdge(nbinsy_vis + 1)) { yy = hist->GetYaxis()->GetBinCenter(nbinsy_vis); }

      int nbinsx = hist->GetXaxis()->GetNbins() + 2;
      int j = hist->GetXaxis()->FindBin(xx);
      int i = hist->GetYaxis()->FindBin(yy);

      return j + nbinsx * i;
    }

    /**
     * Check if the input pdgId is that of a valid charged particle.
     */
    bool isValidPdg(const int pdg)
    {
      return (Const::chargedStableSet.find(pdg) != Const::invalidParticle);
    }

  private:

    /**
      * This map contains - for each charged particle mass hypothesis' pdgId - a 2D (clusterTheta, p) grid of bins as a TH2F.
      * It can be used to lookup the correct xml weightfile in the payload, given a pdgId, clusterTheta and p.
     */
    ClusterThetaPGridByParticle m_grids = {
      { 11, nullptr },
      { 13, nullptr },
      { 211, nullptr },
      { 321, nullptr },
      { 2212, nullptr },
      { 1000010020, nullptr }
    };

    /**
     * This map contains - for each charged particle mass hypothesis' pdgId - a list of (serialized) Weightfile objects to be stored in the payload.
     * The indexing in each vector must reflect the one of the corresponding 'linearised' TH2F histogram conatined in the m_grids map.
     */
    WeightfilesByParticle m_weightfiles = {
      { 11, std::vector<std::string>() },
      { 13, std::vector<std::string>() },
      { 211, std::vector<std::string>() },
      { 321, std::vector<std::string>() },
      { 2212, std::vector<std::string>() },
      { 1000010020, std::vector<std::string>() }
    };

    ClassDef(ChargedPidMVAWeights, 1);
    // 1: first class implementation.
  };

}

