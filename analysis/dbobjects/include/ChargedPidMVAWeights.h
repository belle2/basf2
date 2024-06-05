/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
#include <TH3F.h>
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
    typedef std::map<std::string, std::string> VariablesByAlias; /**< Typedef */

  public:

    /**
     * Default constructor, necessary for ROOT to stream the object.
     */
    ChargedPidMVAWeights() :
      m_energy_unit("energyUnit", Unit::GeV),
      m_ang_unit("angularUnit", Unit::rad),
      m_thetaVarName("clusterTheta"),
      m_implicitNaNmasking(false)
    {};


    /**
     * Specialized constructor.
     */
    ChargedPidMVAWeights(const double& energyUnit, const double& angUnit,
                         const std::string& thetaVarName = "clusterTheta",
                         bool implictNaNmasking = false)
    {
      setEnergyUnit(energyUnit);
      setAngularUnit(angUnit);
      m_thetaVarName = thetaVarName;
      m_implicitNaNmasking = implictNaNmasking;
    }

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
     * Set the 3D (clusterTheta, p, charge) grid representing the categories for which weightfiles are defined.
     * @param clusterThetaBins array of clusterTheta bin edges
     * @param nClusterThetaBins number of clusterTheta bins
     * @param pBins array of p bin edges
     * @param nPBins number of p bins
     * @param chargeBins array of charge bin edges
     * @param nChargeBins number of charge bins
    */
    void setWeightCategories(const double* clusterThetaBins, const int nClusterThetaBins,
                             const double* pBins, const int nPBins,
                             const double* chargeBins, const int nChargeBins)
    {

      m_categories = std::make_unique<TH3F>("clustertheta_p_charge_binsgrid",
                                            ";ECL cluster #theta;p_{lab};Q",
                                            nClusterThetaBins, clusterThetaBins,
                                            nPBins, pBins,
                                            nChargeBins, chargeBins);
    }

    /**
     * Given a particle mass hypothesis' pdgId,
     * store the list of MVA weight files (one for each category) into the payload.
     *
     * @param pdg the particle mass hypothesis' pdgId.
     * @param filepaths a list of xml (root) file paths for several (clusterTheta, p, charge) categories.
     * @param categoryBinCentres a list of <double, double, double> representing the (clusterTheta, p, charge) bin centres.
     *        Used to check consistency of the xml vector indexing w/ the linearised TH3 category map.
     */
    void storeMVAWeights(const int pdg, const std::vector<std::string>& filepaths,
                         const std::vector<std::tuple<double, double, double>>& categoryBinCentres)
    {

      if (!isValidPdg(pdg)) {
        B2FATAL("PDG: " << pdg << " is not that of a valid charged particle! Aborting...");
      }

      unsigned int idx(0);
      for (const auto& path : filepaths) {

        // Index consistency check.
        auto bin_centres_tuple = categoryBinCentres.at(idx);

        auto theta_bin_centre = std::get<0>(bin_centres_tuple);
        auto p_bin_centre = std::get<1>(bin_centres_tuple);
        auto charge_bin_centre = std::get<2>(bin_centres_tuple);

        auto h_idx = getMVAWeightIdx(theta_bin_centre, p_bin_centre, charge_bin_centre);
        if (idx != h_idx) {
          B2FATAL("xml file:\n" << path << "\nindex in input vector:\n" << idx << "\ndoes not correspond to:\n" << h_idx <<
                  "\n, i.e. the linearised index of the 3D bin centered in (clusterTheta, p, charge) = (" << theta_bin_centre << ", " << p_bin_centre
                  << ", " <<
                  charge_bin_centre <<
                  ")\nPlease check how the input xml file list is being filled.");
        }

        Belle2::MVA::Weightfile weightfile;
        if (boost::ends_with(path, ".root")) {
          weightfile = Belle2::MVA::Weightfile::loadFromROOTFile(path);
        } else  if (boost::ends_with(path, ".xml")) {
          weightfile = Belle2::MVA::Weightfile::loadFromXMLFile(path);
        } else {
          B2WARNING("Unknown file extension for file: " << path << ", fallback to xml...");
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
     * Uses the special value of pdg=0 reserved for multi-class mode.
     *
     * @param filepaths a list of xml (root) file paths for several (clusterTheta, p, charge) categories.
     * @param categoryBinCentres a list of <double, double, double> representing the (clusterTheta, p, charge) bin centres.
     *        Used to check consistency of the xml vector indexing w/ the linearised TH3 category map.
     */
    void storeMVAWeightsMultiClass(const std::vector<std::string>& filepaths,
                                   const std::vector<std::tuple<double, double, double>>& categoryBinCentres)
    {
      storeMVAWeights(0, filepaths, categoryBinCentres);
    }


    /**
     * Given a particle mass hypothesis' pdgId,
     * store the list of selection cuts (one for each category) into the payload.
     *
     * @param pdg the particle mass hypothesis' pdgId.
     * @param cutfiles a list of text files w/ cut strings, for each (clusterTheta, p, charge) category.
     *        The format of the cut must comply with the `GeneralCut` syntax.
     * @param categoryBinCentres a list of <double, double, double> representing the (clusterTheta, p, charge) bin centres.
     *        Used to check consistency of the xml vector indexing w/ the linearised TH3 category map.
     */
    void storeCuts(const int pdg, const std::vector<std::string>& cutfiles,
                   const std::vector<std::tuple<double, double, double>>& categoryBinCentres)
    {

      if (!isValidPdg(pdg)) {
        B2FATAL("PDG: " << pdg << " is not that of a valid charged particle! Aborting...");
      }

      unsigned int idx(0);
      for (const auto& cutfile : cutfiles) {

        auto bin_centres_tuple = categoryBinCentres.at(idx);

        auto theta_bin_centre = std::get<0>(bin_centres_tuple);
        auto p_bin_centre = std::get<1>(bin_centres_tuple);
        auto charge_bin_centre = std::get<2>(bin_centres_tuple);

        auto h_idx = getMVAWeightIdx(theta_bin_centre, p_bin_centre, charge_bin_centre);
        if (idx != h_idx) {
          B2FATAL("Cut file:\n" << cutfile << "\nindex in input vector:\n" << idx << "\ndoes not correspond to:\n" << h_idx <<
                  "\n, i.e. the linearised index of the 3D bin centered in (clusterTheta, p, charge) = (" << theta_bin_centre << ", " << p_bin_centre
                  << ", " <<
                  charge_bin_centre <<
                  ")\nPlease check how the input cut file list is being filled.");
        }

        std::ifstream ifs(cutfile);
        std::string cut((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

        // Strip trailing newline.
        cut.erase(std::remove(cut.begin(), cut.end(), '\n'), cut.end());

        m_cuts[pdg].push_back(cut);

        ++idx;
      }

    }

    /**
     * For the multi-class mode,
     * store the list of selection cuts (one for each category) into the payload.
     * Uses the special value of pdg=0 reserved for multi-class mode.
     *
     * @param cutfiles a list of text files w/ cut strings, for each (clusterTheta, p, charge) category.
     *        The format of the cut must comply with the `GeneralCut` syntax.
     * @param categoryBinCentres a list of <double, double, double> representing the (clusterTheta, p, charge) bin centres.
     *        Used to check consistency of the xml vector indexing w/ the linearised TH3 category map.
     */
    void storeCutsMultiClass(const std::vector<std::string>& cutfiles,
                             const std::vector<std::tuple<double, double, double>>& categoryBinCentres)
    {
      storeCuts(0, cutfiles, categoryBinCentres);
    }


    /**
     * Store the map associating variable aliases to variable names knowm to VariableManager.
     *
     * @param aliases a map of (alias, VM variable) pairs. NB: it is supposed to contain all the aliases for every category.
     */
    void storeAliases(const VariablesByAlias& aliases)
    {
      m_aliases = VariablesByAlias(aliases);
    }


    /**
     * Get the raw pointer to the 3D grid representing the categories for which weightfiles are defined.
     * Used just to view the stored data.
     */
    const TH3F* getWeightCategories() const
    {
      return m_categories.get();
    }


    /**
     * Given a particle mass hypothesis' pdgId,
     * get the list of (serialized) MVA weightfiles stored in the payload, one for each category.
     * @param pdg the particle mass hypothesis' pdgId.
     */
    const std::vector<std::string>* getMVAWeights(const int pdg) const
    {
      return &(m_weightfiles.at(pdg));
    }


    /**
     * For the multi-class mode,
     * get the list of (serialized) MVA weightfiles stored in the payload, one for each category.
     * Uses the special value of pdg=0 reserved for multi-class mode.
     */
    const std::vector<std::string>* getMVAWeightsMulticlass() const
    {
      return getMVAWeights(0);
    }


    /**
     * Given a particle mass hypothesis' pdgId,
     * get the list of selection cuts stored in the payload, one for each category.
     * @param pdg the particle mass hypothesis' pdgId.
     */
    const std::vector<std::string>* getCuts(const int pdg) const
    {
      return &(m_cuts.at(pdg));
    }


    /**
     * For the multi-class mode,
     * get the list of selection cuts stored in the payload, one for each category.
     * Uses the special value of pdg=0 reserved for multi-class mode.
     */
    const std::vector<std::string>* getCutsMulticlass() const
    {
      return getCuts(0);
    }


    /**
     * Get the map of unique aliases.
     */
    const VariablesByAlias* getAliases() const
    {
      return &m_aliases;
    }


    /**
     * Get the index of the XML weight file, for a given reconstructed triplet (clusterTheta(theta), p, charge).
     * The index is obtained by linearising the 3D `m_categories` histogram.
     * The same index can be used to look up the correct MVAExpert, Dataset and Cut in the application module,
     * hence we believe it's more useful to return the index rather than a pointer to the weightfile itself.
     * The function also retrieves the 3D bin coordinates.
     * @param theta the particle polar angle (from the cluster, or from the track if no cluster match) in [rad].
     * @param p the particle momentum (from the track) in [GeV/c].
     * @param charge the particle charge (from the track).
     * @param[out] idx_theta the index of the 3D bin along the theta (X) axis.
     * @param[out] idx_p the index of the 3D bin along the p (Y) axis.
     * @param[out] idx_charge the index of the 3D bin along the charge (Z) axis.
     * @return the index of the weightfile of interest from the array of weightfiles.
     */
    unsigned int getMVAWeightIdx(const double& theta, const double& p, const double& charge, int& idx_theta, int& idx_p,
                                 int& idx_charge) const
    {

      if (!m_categories) {
        B2FATAL("No (clusterTheta, p, charge) TH3 grid was found in the DB payload. Most likely, you are using a GT w/ an old payload which is no longer compatible with the DB object class implementation. This should not happen! Abort...");
      }

      int nbins_th = m_categories->GetXaxis()->GetNbins(); // nr. of theta (visible) bins, along X.
      int nbins_p = m_categories->GetYaxis()->GetNbins(); // nr. of p (visible) bins, along Y.

      int glob_bin_idx = findBin(theta / m_ang_unit.GetVal(), p / m_energy_unit.GetVal(), charge);
      m_categories->GetBinXYZ(glob_bin_idx, idx_theta, idx_p, idx_charge);

      // The index of the linearised 3D m_categories.
      // The unit offset is b/c ROOT sets global bin idx also for overflows and underflows.
      return (idx_theta - 1) + nbins_th * ((idx_p - 1) + nbins_p * (idx_charge - 1));
    }

    /**
     * Overloaded method, to be used if not interested in knowing the 3D bin coordinates.
     */
    unsigned int getMVAWeightIdx(const double& theta, const double& p, const double& charge) const
    {
      int idx_theta, idx_p, idx_charge;
      return getMVAWeightIdx(theta, p, charge, idx_theta, idx_p, idx_charge);
    }


    /**
     * Read and dump the payload content from the internal 'matrioska' maps into an XML weightfile for the given set of inputs.
     * Useful for debugging.
     * @param theta the particle polar angle (from the cluster, or from the track if no cluster match) in [rad].
     * @param p the particle momentum (from the track) in [GeV/c].
     * @param charge the particle charge (from the track).
     * @param pdg the particle mass hypothesis' pdgId.
     * @param dump_all dump all information.
     */
    void dumpPayload(const double& theta, const double& p, const double& charge, const int pdg, bool dump_all = false) const
    {

      B2INFO("Dumping payload content for:");
      B2INFO("clusterTheta(theta) = " << theta << " [rad], p = " << p << " [GeV/c], charge = " << charge);

      if (m_categories) {
        std::string filename = "db_payload_chargedpidmva__theta_p_charge_categories.root";
        B2INFO("\tWriting ROOT file w/ TH3F grid that defines categories:" << filename);
        auto f = std::make_unique<TFile>(filename.c_str(), "RECREATE");
        m_categories->Write();
        f->Close();
      } else {
        B2WARNING("\tThe TH3F object that defines categories is a nullptr!");
      }

      for (const auto& [pdgId, weights] : m_weightfiles) {

        if (!dump_all && pdg != pdgId) continue;

        auto idx = getMVAWeightIdx(theta, p, charge);

        auto serialized_weightfile = weights.at(idx);

        std::string filename = "db_payload_chargedpidmva__weightfile_pdg_" + std::to_string(pdgId) +
                               "_glob_bin_" + std::to_string(idx + 1) + ".xml";

        auto cutstr = getCuts(pdgId)->at(idx);

        B2INFO("\tpdgId = " << pdgId);
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
     * Uses the special value of pdg=0 reserved for multi-class mode.
     */
    void dumpPayloadMulticlass(const double& theta, const double& p, const double& charge) const
    {
      dumpPayload(theta, p, charge, 0);
    }


    /**
     * Check if the input pdgId is that of a valid charged particle.
     * An input value of pdg=0 is considered valid, since it's reserved for multi-class mode.
     */
    bool isValidPdg(const int pdg) const
    {
      bool isValid = (Const::chargedStableSet.find(pdg) != Const::invalidParticle) || (pdg == 0);
      return isValid;
    }

    /**
     * Get the name of the polar angle variable.
     */
    std::string getThetaVarName() const
    {
      return m_thetaVarName;
    }


    /**
     * Check flag for implicit NaN masking.
     */
    bool hasImplicitNaNmasking() const
    {
      return m_implicitNaNmasking;
    }


  private:


    /**
     * Find global bin index of the 3D categories histogram for the given (x, y, z) values.
     * This method had to be re-implemented b/c ROOT has no const version of TH1::FindBin() :(
     * @param x value along the x axis.
     * @param y value along the y axis.
     * @param z value along the z axis.
     * @return the global linearised bin index.
    */
    int findBin(const double& x, const double& y, const double& z) const
    {

      int nbinsx_vis = m_categories->GetXaxis()->GetNbins();
      int nbinsy_vis = m_categories->GetYaxis()->GetNbins();
      int nbinsz_vis = m_categories->GetZaxis()->GetNbins();

      double xx = x;
      double yy = y;
      double zz = z;

      // If x, y, z are outside of the 3D grid (visible) range, set their value to
      // fall in the last (first) bin before (after) overflow (underflow).
      if (x < m_categories->GetXaxis()->GetBinLowEdge(1)) { xx = m_categories->GetXaxis()->GetBinCenter(1); }
      if (x >= m_categories->GetXaxis()->GetBinLowEdge(nbinsx_vis + 1)) { xx = m_categories->GetXaxis()->GetBinCenter(nbinsx_vis); }
      if (y < m_categories->GetYaxis()->GetBinLowEdge(1)) { yy = m_categories->GetYaxis()->GetBinCenter(1); }
      if (y >= m_categories->GetYaxis()->GetBinLowEdge(nbinsy_vis + 1)) { yy = m_categories->GetYaxis()->GetBinCenter(nbinsy_vis); }
      if (z < m_categories->GetZaxis()->GetBinLowEdge(1)) { zz = m_categories->GetZaxis()->GetBinCenter(1); }
      if (z >= m_categories->GetZaxis()->GetBinLowEdge(nbinsz_vis + 1)) { zz = m_categories->GetZaxis()->GetBinCenter(nbinsz_vis); }

      int nbinsx = m_categories->GetXaxis()->GetNbins() + 2;
      int nbinsy = m_categories->GetYaxis()->GetNbins() + 2;

      int j = m_categories->GetXaxis()->FindBin(xx);
      int i = m_categories->GetYaxis()->FindBin(yy);
      int k = m_categories->GetZaxis()->FindBin(zz);

      return j + nbinsx * (i + nbinsy * k);
    }


  private:


    TParameter<double> m_energy_unit; /**< The energy unit used for defining the bins grid. */
    TParameter<double> m_ang_unit;    /**< The angular unit used for defining the bins grid. */
    std::string
    m_thetaVarName; /**< The name of the polar angle variable used in the MVA categorisation. Must be a string that can be parsed by the VariableManager. */
    bool m_implicitNaNmasking; /**< Flag to indicate whether the MVA variables have been NaN-masked directly in the weightfiles. */


    /**
     * A 3D histogram whose bins represent the categories for which XML weight files are defined.
     * It is used to lookup the correct file in the payload, given a reconstructed set of (clusterTheta(theta), p, charge).
     */
    std::unique_ptr<TH3F> m_categories;


    /**
     * For each charged particle mass hypothesis' pdgId,
     * this map contains a list of (serialized) Weightfile objects to be stored in the payload.
     * Each weightfile in the list corresponds to a 3D category.
     * The indexing in each vector must reflect the one of the corresponding 'linearised' TH3F histogram contained in the m_grids map.
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
     * The indexing in each vector must reflect the one of the corresponding 'linearised' TH3F histogram contained in the m_grids map.
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


    /**
     * A map that associates variable aliases used in the MVA training to variable names known to the VariableManager.
     */
    VariablesByAlias m_aliases;


    ClassDef(ChargedPidMVAWeights, 10);
    /**< 10. Add name of polar angle variable used for categorisation, and a boolean flag to check if implicit NaN masking is set in the input data. */
    /**< 9. Add map of variable aliases and original basf2 vars. */
    /**< 8. Use unique_ptr for m_categories. */
    /**< 7. Use double instead of float in tuple. */
    /**< 6. Introduce charge bin in the parametrisation. */
    /**< 5. remove 2D grid dependence on pdgId, add multi-class support, define enum for valid training modes */
    /**< 4. add cuts map. */
    /**< 3. add overloaded getMVAWeightIdx. */
    /**< 2: add energy/angular units. */
    /**< 1: first class implementation. */
  };

}
