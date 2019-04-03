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
#include <TParameter.h>
#include <TFile.h>

//BOOST
#include <boost/algorithm/string/predicate.hpp>
#include <boost/property_tree/xml_parser.hpp>

//C++
#include <unordered_map>
#include <fstream>


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
    ChargedPidMVAWeights() :
      m_energy_unit("energyUnit", Unit::rad),
      m_ang_unit("angularUnit", Unit::GeV)
    {};


    /**
     * Destructor.
     */
    ~ChargedPidMVAWeights() {};


    /**
     * Set the energy unit to ensure consistency w/ the one used to define the bins grid.
     */
    void setEnergyUnit(const double& unit) { m_energy_unit.SetVal(unit); }


    /**
     * Set the angular unit to ensure consistency w/ the one used to define the bins grid.
     */
    void setAngularUnit(const double& unit) { m_ang_unit.SetVal(unit); }


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
     * Get the index of the weightfile of interest in the list of weightfiles, for a given particle mass hypothesis, clusterTheta, and p.
     * The index is obtained by linearising the 2D histogram representing the (clusterTheta, p) bins grid.
     * The same index can be used to look up the correct MVA Expert and Dataset in the application module,
     * hence we believe it's more useful to return the index rather than a pointer to the weightfile itself.
     * The function also retrieves the (clusterTheta, p) bin coordinates.
     * @param part the particle mass hypothesis.
     * @param theta the particle polar angle (from the ECL cluster) in [rad].
     * @param p the particle momentum (from the track) in [GeV/c].
     * @param[out] jth the index of the (theta, p) bin along the theta (X) axis.
     * @param[out] ip the index of the (theta, p) bin along the p (Y) axis.
     * @return the index of the weightfile of interest from the array of weightfiles.
    */
    unsigned int getMVAWeightIdx(const Const::ChargedStable& part, const double& theta, const double& p, int& jth, int& ip) const
    {

      const TH2F* grid = m_grids.at(part.getPDGCode());

      if (!grid) {
        B2FATAL("No (clusterTheta, p) TH2 bins grid was found in the DB payload for pdg: " << part.getPDGCode() <<
                ". This should not happen! Abort...");
      }

      int nbins_th = grid->GetXaxis()->GetNbins(); // nr. of theta (visible) bins, along X.

      int glob_bin_idx = findBin(grid, theta / m_ang_unit.GetVal(), p / m_energy_unit.GetVal());
      int k;
      grid->GetBinXYZ(glob_bin_idx, jth, ip, k);

      // The index of the linearised 2D (theta,p) grid.
      // The unit offset is b/c ROOT sets global bin idx also for overflows and underflows.
      return (jth - 1) + nbins_th * (ip - 1);

    }

    /**
     * Overloaded method, to be used if not interested in knowing the 2D (clusterTheta, p) bin coordinates.
     */
    unsigned int getMVAWeightIdx(const Const::ChargedStable& part, const double& theta, const double& p) const
    {
      int jth, ip;
      return getMVAWeightIdx(part, theta, p, jth, ip);
    }

    /**
     * Read and dump the payload content from the internal 'matrioska' maps into an xml weightfile for the given set of inputs. Useful for debugging.
     * @param theta the particle polar angle (from the ECL cluster) in [rad].
     * @param p the particle momentum (from the track) in [GeV/c].
     * @param pdg the particle mass hypothesis' pdgId.
     */
    void dumpPayload(const double& theta, const double& p, const int pdg = 0) const
    {

      B2INFO("Dumping payload content for...");
      B2INFO("-) clusterTheta = " << theta << " [rad]");
      B2INFO("-) p = " << p << " [GeV/c]");

      for (const auto& [pdgId, grid] : m_grids) {

        if (pdg && pdg != pdgId) continue;

        B2INFO("-) pdgId = " << pdgId);

        if (grid) {
          std::string filename = "db_payload_chargedpidmva__theta_p_grid_pdg_" + std::to_string(pdgId) + ".root";
          B2INFO("\tWriting ROOT file w/ (clusterTheta, p) bins grid: " << filename);
          auto f = std::make_unique<TFile>(filename.c_str(), "RECREATE");
          grid->Write();
          f->Close();
        } else {
          B2WARNING("\tTH2 for pdg " << pdg << "is a nullptr!");
        }
      }

      for (const auto& [pdgId, weights] : m_weightfiles) {

        if (pdg && pdg != pdgId) continue;

        auto idx = getMVAWeightIdx(Const::ChargedStable(pdg), theta, p);

        auto serialized_weightfile = weights.at(idx);

        std::string filename = "db_payload_chargedpidmva__weightfile_pdg_" + std::to_string(pdgId) + "_" + std::to_string(idx) + ".xml";

        B2INFO("\tWriting weight file: " << filename);

        std::ofstream weightfile;
        weightfile.open(filename.c_str(), std::ios::out);
        weightfile << serialized_weightfile << std::endl;
        weightfile.close();

      }

    };

    /**
     * Check if the input pdgId is that of a valid charged particle.
     */
    bool isValidPdg(const int pdg) const
    {
      return (Const::chargedStableSet.find(pdg) != Const::invalidParticle);
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

  private:

    TParameter<double> m_energy_unit; /**< The energy unit used for defining the bins grid. */
    TParameter<double> m_ang_unit;    /**< The angular unit used for defining the bins grid. */

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

    ClassDef(ChargedPidMVAWeights, 3);
    // 1: first class implementation.
    // 2: add energy/angular units.
    // 3. add overloaded getMVAWeightIdx.
  };

}

