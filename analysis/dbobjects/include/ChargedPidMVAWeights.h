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

// MVA
#include <mva/interface/Weightfile.h>

// ROOT
#include <TObject.h>
#include <TH2F.h>
#include <TParameter.h>
#include <TFile.h>

//BOOST
#include <boost/algorithm/string/predicate.hpp>


namespace Belle2 {

  /**
   * Class to contain the payload of MVA weightfiles needed for charged particle identification.
   */
  class ChargedPidMVAWeights : public TObject {

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
     * A (strongly-typed) enumerator identifier for each valid MVA training mode.
     */
    enum class ChargedPidMVATrainingMode : unsigned int {
      /** Binary classification */
      c_Classification = 0,
      /** Multi-class classification */
      c_Multiclass = 1,
      /** Binary classification, ECL only */
      c_ECL_Classification = 2,
      /** Multi-class classification, ECL only */
      c_ECL_Multiclass = 3,
      /** Binary classification, including PSD */
      c_PSD_Classification = 4,
      /** Multi-class classification, including PSD */
      c_PSD_Multiclass = 5,
      /** Binary classification, ECL only, including PSD */
      c_ECL_PSD_Classification = 6,
      /** Multi-class classification, ECL only, including PSD */
      c_ECL_PSD_Multiclass = 7
    };


    /**
     * Set the energy unit to ensure consistency w/ the one used to define the bins grid.
     */
    void setEnergyUnit(const double& unit) { m_energy_unit.SetVal(unit); }


    /**
     * Set the angular unit to ensure consistency w/ the one used to define the bins grid.
     */
    void setAngularUnit(const double& unit) { m_ang_unit.SetVal(unit); }


    /**
     * Set the 2D (clusterTheta, p) grid representing the categories for which weightfiles are defined.
     * @param h the 2D histogram.
    */
    void setWeightCategories(TH2F* h)
    {
      m_categories = h;
    }

    /**
     * Given a particle mass hypothesis' pdgId,
     * store the list of MVA weight files (one for each category) into the payload.
     *
     * @param pdg the particle mass hypothesis' pdgId.
     * @param filepaths a list of xml (root) file paths for several (clusterTheta,p) categories.
     * @param categoryBinCentres a list of pair<float, float> representing the (clusterTheta,p) bin centres.
     *        Used to check consistency of the xml vector indexing w/ the linearised TH2 category map.
     */
    void storeMVAWeights(const int pdg, const std::vector<std::string>& filepaths,
                         const std::vector<std::pair<float, float>>& categoryBinCentres)
    {

      if (!isValidPdg(pdg)) {
        B2FATAL("PDG: " << pdg << " is not that of a valid charged particle! Aborting...");
      }

      unsigned int idx(0);
      for (const auto& path : filepaths) {

        // Index consistency check.
        auto theta_p = categoryBinCentres.at(idx);
        auto h_idx = getMVAWeightIdx(theta_p.first, theta_p.second);
        if (idx != h_idx) {
          B2FATAL("xml file:\n" << path << "\nindex in input vector:\n" << idx << "\ndoes not correspond to:\n" << h_idx <<
                  "\n, i.e. the linearised index of the 2D bin centered in (clusterTheta, p) = (" << theta_p.first << ", " << theta_p.second <<
                  ")\nPlease check how the input xml file list is being filled.");
        }

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

        ++idx;
      }

    }


    /**
     * For the multi-class mode,
     * store the list of MVA weight files (one for each category) into the payload.
     *
     * @param filepaths a list of xml (root) file paths for several (clusterTheta,p) categories.
     * @param categoryBinCentres a list of pair<float, float> representing the (clusterTheta,p) bin centres.
     *        Used to check consistency of the xml vector indexing w/ the linearised TH2 category map.
     */
    void storeMVAWeightsMultiClass(const std::vector<std::string>& filepaths,
                                   const std::vector<std::pair<float, float>>& categoryBinCentres)
    {
      storeMVAWeights(0, filepaths, categoryBinCentres);
    }


    /**
     * Given a particle mass hypothesis' pdgId,
     * store the list of selection cuts (one for each category) into the payload.
     *
     * @param pdg the particle mass hypothesis' pdgId.
     * @param cutfiles a list of text files w/ cut strings, for each (clusterTheta, p) category.
     *        The format of the cut must comply with the `GeneralCut` syntax.
     * @param categoryBinCentres a list of pair<float, float> representing the (clusterTheta,p) bin centres.
     *        Used to check consistency of the xml vector indexing w/ the linearised TH2 category map.
     */
    void storeCuts(const int pdg, const std::vector<std::string>& cutfiles,
                   const std::vector<std::pair<float, float>>& categoryBinCentres)
    {

      if (!isValidPdg(pdg)) {
        B2FATAL("PDG: " << pdg << " is not that of a valid charged particle! Aborting...");
      }

      unsigned int idx(0);
      for (const auto& cutfile : cutfiles) {

        // Index consistency check.
        auto theta_p = categoryBinCentres.at(idx);
        auto h_idx = getMVAWeightIdx(theta_p.first, theta_p.second);
        if (idx != h_idx) {
          B2FATAL("Cut file:\n" << cutfile << "\nindex in input vector:\n" << idx << "\ndoes not correspond to:\n" << h_idx <<
                  "\n, i.e. the linearised index of the 2D bin centered in (clusterTheta, p) = (" << theta_p.first << ", " << theta_p.second <<
                  ")\nPlease check how the input cut file list is being filled.");
        }

        std::ifstream ifs(cutfile);
        std::string cut((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

        // Strip trailing newline.
        cut.erase(std::remove(cut.begin(), cut.end(), '\n'), cut.end());

        // Conditional expression separator must use square brackets in basf2.
        std::replace(cut.begin(), cut.end(), '(', '[');
        std::replace(cut.begin(), cut.end(), ')', ']');

        m_cuts[pdg].push_back(cut);

        ++idx;
      }

    }

    /**
     * For the multi-class mode,
     * store the list of selection cuts (one for each category) into the payload.
     *
     * @param cutfiles a list of text files w/ cut strings, for each (clusterTheta, p) category.
     *        The format of the cut must comply with the `GeneralCut` syntax.
     * @param categoryBinCentres a list of pair<float, float> representing the (clusterTheta,p) bin centres.
     *        Used to check consistency of the xml vector indexing w/ the linearised TH2 category map.
     */
    void storeCutsMultiClass(const std::vector<std::string>& cutfiles, const std::vector<std::pair<float, float>>& categoryBinCentres)
    {
      storeCuts(0, cutfiles, categoryBinCentres);
    }


    /**
     * Given a particle mass hypothesis' pdgId,
     * get the list of (serialized) MVA weightfiles stored in the payload, one for each (clusterTheta, p) category.
     * @param pdg the particle mass hypothesis' pdgId.
     */
    const std::vector<std::string>* getMVAWeights(const int pdg) const
    {
      return &(m_weightfiles.at(pdg));
    }


    /**
     * For the multi-class mode,
     * get the list of (serialized) MVA weightfiles stored in the payload, one for each (clusterTheta, p) category.
     */
    const std::vector<std::string>* getMVAWeightsMulticlass() const
    {
      return getMVAWeights(0);
    }


    /**
     * Given a particle mass hypothesis' pdgId,
     * get the list of selection cuts stored in the payload, one for each (clusterTheta, p) category.
     * @param pdg the particle mass hypothesis' pdgId.
     * @param pdg the particle mass hypothesis' pdgId.
     */
    const std::vector<std::string>* getCuts(const int pdg) const
    {
      return &(m_cuts.at(pdg));
    }


    /**
     * For the multi-class mode,
     * get the list of selection cuts stored in the payload, one for each (clusterTheta, p) category.
     */
    const std::vector<std::string>* getCutsMulticlass() const
    {
      return getCuts(0);
    }


    /**
     * Get the index of the XML weight file, for a given reconstructed pair (clusterTheta, p).
     * The index is obtained by linearising the 2D `m_categories` histogram.
     * The same index can be used to look up the correct MVAExpert, Dataset and Cut in the application module,
     * hence we believe it's more useful to return the index rather than a pointer to the weightfile itself.
     * The function also retrieves the (clusterTheta, p) bin coordinates.
     * @param theta the particle polar angle (from the ECL cluster) in [rad].
     * @param p the particle momentum (from the track) in [GeV/c].
     * @param[out] jth the index of the (theta, p) bin along the theta (X) axis.
     * @param[out] ip the index of the (theta, p) bin along the p (Y) axis.
     * @return the index of the weightfile of interest from the array of weightfiles.
    */
    unsigned int getMVAWeightIdx(const double& theta, const double& p, int& jth, int& ip) const
    {

      if (!m_categories) {
        B2FATAL("No (clusterTheta, p) TH2 grid was found in the DB payload. This should not happen! Abort...");
      }

      int nbins_th = m_categories->GetXaxis()->GetNbins(); // nr. of theta (visible) bins, along X.

      int glob_bin_idx = findBin(m_categories, theta / m_ang_unit.GetVal(), p / m_energy_unit.GetVal());
      int k;
      m_categories->GetBinXYZ(glob_bin_idx, jth, ip, k);

      // The index of the linearised 2D (theta,p) m_categories.
      // The unit offset is b/c ROOT sets global bin idx also for overflows and underflows.
      return (jth - 1) + nbins_th * (ip - 1);

    }


    /**
     * Overloaded method, to be used if not interested in knowing the 2D (clusterTheta, p) bin coordinates.
     */
    unsigned int getMVAWeightIdx(const double& theta, const double& p) const
    {
      int jth, ip;
      return getMVAWeightIdx(theta, p, jth, ip);
    }


    /**
     * Read and dump the payload content from the internal 'matrioska' maps into an XML weightfile for the given set of inputs.
     * Useful for debugging.
     * @param theta the particle polar angle (from the ECL cluster) in [rad].
     * @param p the particle momentum (from the track) in [GeV/c].
     * @param pdg the particle mass hypothesis' pdgId.
     */
    void dumpPayload(const double& theta, const double& p, const int pdg, bool dump_all = false) const
    {

      B2INFO("Dumping payload content for...");
      B2INFO("-) clusterTheta = " << theta << " [rad]");
      B2INFO("-) p = " << p << " [GeV/c]");

      if (m_categories) {
        std::string filename = "db_payload_chargedpidmva__clustertheta_p_categories.root";
        B2INFO("\tWriting ROOT file w/ (clusterTheta, p) TH2F grid that defines categories:" << filename);
        auto f = std::make_unique<TFile>(filename.c_str(), "RECREATE");
        m_categories->Write();
        f->Close();
      } else {
        B2WARNING("\tThe TH2F object that defines categories is a nullptr!");
      }

      for (const auto& [pdgId, weights] : m_weightfiles) {

        if (!dump_all && pdg != pdgId) continue;

        B2INFO("-) pdgId = " << pdgId);

        auto idx = getMVAWeightIdx(theta, p);

        auto serialized_weightfile = weights.at(idx);

        std::string filename = "db_payload_chargedpidmva__weightfile_pdg_" + std::to_string(pdgId) +
                               "_glob_bin_" + std::to_string(idx + 1) + ".xml";

        auto cutstr = getCuts(pdgId)->at(idx);

        B2INFO("\tCut: " << cutstr);
        B2INFO("\tWriting weight file: " << filename);

        std::ofstream weightfile;
        weightfile.open(filename.c_str(), std::ios::out);
        weightfile << serialized_weightfile << std::endl;
        weightfile.close();

      }

    };


    /**
     * Special version for multi-class mode.
     */
    void dumpPayloadMulticlass(const double& theta, const double& p) const
    {
      dumpPayload(theta, p, 0);
    }


    /**
     * Check if the input pdgId is that of a valid charged particle.
     * An input value of  pdg=0 is considered valid, since it's reserved for multi-class mode.
     */
    bool isValidPdg(const int pdg) const
    {
      bool isValid = (Const::chargedStableSet.find(pdg) != Const::invalidParticle) || (pdg == 0);
      return isValid;
    }


  private:


    /**
     * Find global bin index of a 2D histogram for the given (x, y) values.
     * This method had to be re-implemented b/c ROOT has no const version of TH1::FindBin() :(
     * @param x value along the x axis.
     * @param y value along the y axis.
     * @return the global linearised bin index.
    */
    int findBin(const TH2F* h, const double& x, const double& y) const
    {

      int nbinsx_vis = h->GetXaxis()->GetNbins();
      int nbinsy_vis = h->GetYaxis()->GetNbins();

      double xx = x;
      double yy = y;

      // If x, y are outside of the 2D hogram grid (visible) range, set their value to
      // fall in the last (first) bin before (after) overflow (underflow).
      if (x < h->GetXaxis()->GetBinLowEdge(1))             { xx = h->GetXaxis()->GetBinCenter(1); }
      if (x >= h->GetXaxis()->GetBinLowEdge(nbinsx_vis + 1)) { xx = h->GetXaxis()->GetBinCenter(nbinsx_vis); }
      if (y < h->GetYaxis()->GetBinLowEdge(1))             { yy = h->GetYaxis()->GetBinCenter(1); }
      if (y >= h->GetYaxis()->GetBinLowEdge(nbinsy_vis + 1)) { yy = h->GetYaxis()->GetBinCenter(nbinsy_vis); }

      int nbinsx = h->GetXaxis()->GetNbins() + 2;
      int j = h->GetXaxis()->FindBin(xx);
      int i = h->GetYaxis()->FindBin(yy);

      return j + nbinsx * i;
    }


  private:


    TParameter<double> m_energy_unit; /**< The energy unit used for defining the bins grid. */
    TParameter<double> m_ang_unit;    /**< The angular unit used for defining the bins grid. */


    /**
     * A 2D (clusterTheta, p) histogram whose bins represent the categories for which XML weight files are defined.
      * It is used to lookup the correct file in the payload, given a reconstructed pair (clusterTheta, p).
     */
    TH2F* m_categories = nullptr;


    /**
     * For each charged particle mass hypothesis' pdgId,
     * this map contains a list of (serialized) Weightfile objects to be stored in the payload.
     * Each weightfile in the list corresponds to a (clusterTheta, p) category.
     * The indexing in each vector must reflect the one of the corresponding 'linearised' TH2F histogram contained in the m_grids map.
     *
     * The dummy pdgId=0 key is reserved for multi-class, where a unique signal hypothesis is not defined.
     */
    WeightfilesByParticle m_weightfiles = {
      { 0, std::vector<std::string>() },
      { Const::electron.getPDGCode(), std::vector<std::string>() },
      { Const::muon.getPDGCode(), std::vector<std::string>() },
      { Const::pion.getPDGCode(), std::vector<std::string>() },
      { Const::kaon.getPDGCode(), std::vector<std::string>() },
      { Const::proton.getPDGCode(), std::vector<std::string>() },
      { Const::deuteron.getPDGCode(), std::vector<std::string>() }
    };


    /**
     * For each charged particle mass hypothesis' pdgId,
     * this map contains a list of selection cuts to be stored in the payload.
     * To each Weightfile (i.e., category) corresponds a cut.
     * The indexing in each vector must reflect the one of the corresponding 'linearised' TH2F histogram contained in the m_grids map.
     *
     * The dummy pdgId=0 key is reserved for multi-class, where a unique signal hypothesis is not defined.
     */
    WeightfilesByParticle m_cuts = {
      { 0, std::vector<std::string>() },
      { Const::electron.getPDGCode(), std::vector<std::string>() },
      { Const::muon.getPDGCode(), std::vector<std::string>() },
      { Const::pion.getPDGCode(), std::vector<std::string>() },
      { Const::kaon.getPDGCode(), std::vector<std::string>() },
      { Const::proton.getPDGCode(), std::vector<std::string>() },
      { Const::deuteron.getPDGCode(), std::vector<std::string>() }
    };


    ClassDef(ChargedPidMVAWeights, 5);
    /**< 5. remove 2D grid dependence on pdgId, add multi-class support, define enum for valid training modes */
    /**< 4. add cuts map. */
    /**< 3. add overloaded getMVAWeightIdx. */
    /**< 2: add energy/angular units. */
    /**< 1: first class implementation. */
  };

}
