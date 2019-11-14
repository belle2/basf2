/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Parameters of the ECL PDFs for charged particles.                      *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marco Milesi (marco.milesi@unimelb.edu.au)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <unordered_map>

#include <TObject.h>
#include <TParameter.h>
#include <TH2F.h>
#include <TF1.h>

namespace Belle2 {

  /** Class representing the DB payload w/ information about ECL PDF parameters for a set of particle hypotheses signed pdgId values. */
  class ECLChargedPidPDFs: public TObject {

    typedef std::unordered_map<int, TH2F> BinsHistoByParticle; /**< Typedef */
    typedef std::unordered_map<int, TF1> PdfsByBinIdxs; /**< Typedef */
    typedef std::unordered_map<int, PdfsByBinIdxs > PdfsMapByParticle; /**< Typedef */

  public:

    /** Default constructor */
    ECLChargedPidPDFs() :
      m_energy_unit("energyUnit", Unit::rad),
      m_ang_unit("angularUnit", Unit::GeV),
      m_binshisto(),
      m_pdfsmap()
    {};

    /** Destructor */
    ~ECLChargedPidPDFs() {};

    /** Print out the content of the PDf map. Useful for debugging.
     */
    enum class InputVar : unsigned int {
      /** Null var */
      c_NONE = 0,
      /** E1/E9 */
      c_E1E9 = 1,
      /** E9/E21 */
      c_E9E21 = 2,
      /** Second moment */
      c_S2 = 3,
      /** Energy of maxE shower */
      c_E = 4,
      /** E/p */
      c_EoP = 5,
      /** Zernike moment 40 */
      c_Z40 = 6,
      /** Zernike moment 51 */
      c_Z51 = 7,
      /** Zernike MVA */
      c_ZMVA = 8,
      /** PSD MVA */
      c_PSDMVA = 9,
      /** DeltaL (track depth) */
      c_DeltaL = 10,
      /** Lateral shower shape */
      c_LAT = 11
    };

    /**
     * Class to hold parameters needed to perform pre-processing of input variables (e.g., gaussianisation, decorrelation) to build a multi-dimensional likelihood model.
     * All matrices are stored in a linearised form, as vectors.
     */
    class VarTransfoSettings {
    public:

      VarTransfoSettings() {}; /** Constructor */
      ~VarTransfoSettings() {}; /** Destructor */

      bool doTransfo = false; /** To be toggled on if the class instance will be effectively configured
          (e.g., it won't be done if classifier is univariate in a given category). */
      int nVars; /** Number of variables. */
      std::string classPath; /** Path of the class used to get the variables transfo. Useful for debugging. */
      std::vector<int> nDivisions; /** Number of steps in which each variable range is sub-divided. */
      int nDivisionsMax; /** Maximal number of steps, across all variables */
      std::vector<double> cumulDist; /** Cumulative density function at each step. */
      std::vector<double> x; /** Variable value at each step. */
      std::vector<double> covMatrix; /** Variables covariance matrix. */

      unsigned int ip; /** p bin index */
      unsigned int jth; /** theta bin index */
      unsigned int gbin; /** Global bin corresponding to (jth,ip) */
    };

    typedef std::unordered_map<int, TH2F> BinsHistoByParticle; /**< Typedef */
    typedef std::unordered_map<InputVar, TF1*> PdfsByVariable; /**< Typedef */
    typedef std::unordered_map<int, PdfsByVariable > PdfsMapByBinIdxs; /**< Typedef */
    typedef std::unordered_map<int, PdfsMapByBinIdxs > PdfsMapByParticle; /**< Typedef */
    typedef std::unordered_map<int, std::vector<InputVar> > VariablesMapByBinIdxs; /**< Typedef */
    typedef std::unordered_map<int, VariablesMapByBinIdxs > VariablesMapByParticle; /**< Typedef */
    typedef std::unordered_map<int, VarTransfoSettings> VTSMapByBinIdxs; /**< Typedef */
    typedef std::unordered_map<int, VTSMapByBinIdxs > VTSMapByParticle; /**< Typedef */

    /**
     * Set the names of the input variables for which PDFs are stored for a given category (hypo, p, theta).
     @param pdg the particle hypothesis' signed pdgId.
     @param i the index along the 2D grid Y axis (rows) --> p.
     @param j the index along the 2D grid X axis (cols) --> theta.
     @param vars the list of variables. The variable ID is not a string, but an enum code integer as defined in InputVar.
     */
    void setVars(const int pdg, const unsigned int i, const unsigned int j,
                 const std::vector<InputVar>& vars)
    {
      auto ji = getBinsHist(pdg)->GetBin(j, i);

      m_variablesmap_bybinidxs[ji] = vars;

      m_variablesmap[pdg] = m_variablesmap_bybinidxs;

    }

    /**
     * Getter for list of input variables (enums) for which PDFs are stored for a given category (hypo, p, theta).
     @param pdg the particle hypothesis' signed pdgId.
     @param p the reconstructed momentum of the particle's track.
     @param theta the reconstructed polar angle of the particle's cluster.
     */
    const std::vector<InputVar>* getVars(const int pdg, const double& p, const double& theta) const
    {

      auto gbin = findBin(getBinsHist(pdg), theta, p);

      return &(m_variablesmap.at(pdg).at(gbin));

    }

    /**
     * Print out the content of the internal 'matrioska' maps. Useful for debugging.
     */
    void printPdfMap(const int pdg = 0, const double& p = -1.0, const double& theta = -999.0,
                     const InputVar varid = InputVar::c_NONE) const
    {

      std::cout << "Printing PDF info: " << std::endl;
      if (pdg) std::cout << "-) pdgId = " << pdg << std::endl;
      if (p != -1.0) std::cout << "-) p = " << p << " [GeV/c]" << std::endl;
      if (theta != -999.0) std::cout << "-) theta = " << theta << " [rad]" << std::endl;
      if (varid != InputVar::c_NONE) std::cout << "-) varid = " << static_cast<unsigned int>(varid) << std::endl;

      int x, y, z;
      for (const auto& pair0 : m_pdfsmap) {

        if (pdg && pdg != pair0.first) continue;

        for (const auto& pair1 : pair0.second) {

          auto ji(-1);
          if (p != -1.0 && theta != -999.0) {
            const TH2F* h = getBinsHist(pdg);
            ji = findBin(h, theta, p);
            h->GetBinXYZ(ji, x, y, z);
          }
          if (ji > 0 && ji != pair1.first) continue;

          std::cout << "\tglobal_bin_idx: " << pair1.first << " (" << x << "," << y << ")" << std::endl;

          for (const auto& pair2 : pair1.second) {
            if (varid != InputVar::c_NONE && varid != pair2.first) continue;
            std::cout << "\t\tvarid: " << static_cast<unsigned int>(varid) << ", TF1: " << pair2.second->GetName() << std::endl;
          }

        }
      }
    };

    /** Set the energy unit to be consistent w/ the one used in the fit.
     */
    void setEnergyUnit(const double& unit) { m_energy_unit.SetVal(unit); }

    /** Set the angular unit to be consistent w/ the one used in the fit.
     */
    void setAngularUnit(const double& unit) { m_ang_unit.SetVal(unit); }

    /**
     * Set the 2D (theta, p) grid that defines the categories for which PDFs are defined.
     @param hypo the particle hypothesis' signed pdgId.
     @param binhist the 2D histogram w/ the bin grid.
    */
    void setBinsHist(const int pdg, TH2F binhist)
    {
      m_binshisto.emplace(pdg, binhist);
    };

    /** Return 2D bins grid histogram for the given particle and charge hypothesis.
    @param hypo the particle hypothesis' signed pdgId.
     */
    const TH2F* getBinsHist(const int pdg) const
    {
      return &m_binshisto.at(pdg);
    }

    /**
     * Fills the internal maps of this class' instance for a given category (hypo, p, theta).
     @param pdg the particle hypothesis' signed pdgId.
     @param i the index along the 2D grid Y axis (rows) --> p.
     @param j the index along the 2D grid X axis (cols) --> theta.
     @param varid the observable's enum identifier.
     @param pdf the pdf object.
    */
    void add(const int pdg, const unsigned int i, const unsigned int j, const InputVar varid, TF1* pdf)
    {
      auto ij = getBinsHist(pdg)->GetBin(j, i);
      m_pdfsbybinidxs.emplace(ij, pdf);
    };


    /** Set the PDF map :
    @param pdg the particle hypothesis' signed pdgId.
     */
    void setPDFsMap(const int pdg)
    {
      m_pdfsmap.emplace(pdg, m_pdfsbybinidxs);
      m_pdfsbybinidxs.clear(); // Don't forget to clear the internal map for the next call!
    };

    /**
     * Return the PDF of this observable for this candidate's reconstructed (p, theta), under the given particle hypothesis.
     @param pdg the particle hypothesis' signed pdgId.
     @param p the reconstructed momentum of the candidate's track.
     @param theta the reconstructed polar angle of the candidate's cluster.
     @param varid the observable's enum identifier.
    */
    const TF1* getPdf(const int pdg, const double& p, const double& theta, const InputVar varid) const
    {

      const TH2F* binshist = getBinsHist(pdg);

      double pp = p / m_energy_unit.GetVal();
      double th = TMath::Abs(theta) / m_ang_unit.GetVal();

      int nbinsp = binshist->GetNbinsY();

      // If p is outside of the 2D histogram grid (visible) range, set its value to
      // fall in the last (first) bin before (after) overflow (underflow).
      if (pp < binshist->GetYaxis()->GetBinLowEdge(1))         { pp = binshist->GetYaxis()->GetBinCenter(1); }
      if (pp >= binshist->GetYaxis()->GetBinLowEdge(nbinsp + 1)) { pp = binshist->GetYaxis()->GetBinCenter(nbinsp); }

      int gbin = findBin(binshist, th, pp);

      int x, y, z;
      binshist->GetBinXYZ(gbin, x, y, z);
      B2DEBUG(20, "pdgId = " << pdg);
      B2DEBUG(20, "Angular unit: " <<  m_ang_unit.GetVal());
      B2DEBUG(20, "Energy unit: " << m_energy_unit.GetVal());
      B2DEBUG(20, "abs(Theta) = " << th);
      B2DEBUG(20, "P = " << pp);
      B2DEBUG(20, "gbin = " << gbin << ", (" << y << "," << x << ")");

      return &(m_pdfsmap.at(pdg).at(gbin));
    }

  private:

    /** Find global bin index for the given x,y values.
    This method was re-implemented b/c ROOT has no const version of TH1::FindBin() :(
     */
    int findBin(const TH2F* hist, const double& x, const double& y) const
    {
      int nx   = hist->GetXaxis()->GetNbins() + 2;
      int binx = hist->GetXaxis()->FindBin(x);
      int biny = hist->GetYaxis()->FindBin(y);

      return  binx + nx * biny;
    }

    /**
     * Setup the variable transformations a given category (hypo, p, theta) needed to build a multi-dimensional likelihood.
     @param pdg the particle hypothesis' signed pdgId.
     @param i the index along the 2D grid Y axis (rows) --> p.
     @param j the index along the 2D grid X axis (cols) --> theta.
     @param nvars the number of input variables used to build the model.
     @param classPath (for debugging) full path to the file containing the TMVA standalone class used to get the variables transformation parameters.
     @param cumulDist the value of the variable's cumulative density function at each step.
     @param x the value of the variable at each step.
     @param nDivisions the number of divisions (steps) of the variable's range.
     @param covMatrix the variables' inverse square-root covariance matrix.
    */
    void storeVarsTransfoSettings(const int pdg, const unsigned int i, const unsigned int j,
                                  const int nVars,
                                  const std::string& classPath = "",
                                  const std::vector<int>& nDivisions = std::vector<int>(),
                                  const std::vector<double>& cumulDist = std::vector<double>(),
                                  const std::vector<double>& x = std::vector<double>(),
                                  const std::vector<double>& covMatrix = std::vector<double>())
    {

      VarTransfoSettings vts;

      vts.doTransfo = (nVars > 1); /** No transformation will be attempted if univariate! */
      vts.nVars = nVars;
      vts.classPath = classPath;
      vts.nDivisionsMax = (!nDivisions.empty()) ? *(std::max_element(std::begin(nDivisions), std::end(nDivisions))) : 0;
      vts.nDivisions = nDivisions;
      vts.cumulDist = cumulDist;
      vts.x = x;
      vts.covMatrix = covMatrix;

      auto ji = getBinsHist(pdg)->GetBin(j, i);
      vts.gbin = ji;
      vts.ip   = i;
      vts.jth  = j;

      m_vtsmap_bybinidxs[ji] = vts;

      m_vtsmap[pdg] = m_vtsmap_bybinidxs;

    }

    /**
     * Getter for variable transformation settings.
     @param pdg the particle hypothesis' signed pdgId.
     @param p the reconstructed momentum of the candidate's track.
     @param theta the reconstructed polar angle of the candidate's cluster.
     */
    const VarTransfoSettings* getVTS(const int pdg, const double& p, const double& theta) const
    {

      auto gbin = findBin(getBinsHist(pdg), theta, p);

      return &(m_vtsmap.at(pdg).at(gbin));

    }

  private:

    TParameter<double> m_energy_unit; /**< The energy unit used for the binning. */
    TParameter<double> m_ang_unit;    /**< The angular unit used for the binning. */

    /**
     * Internal map.
     * The key corresponds to the particle hypothesis' signed pdgId.
     * The mapped value is a VariablesMapByBinIdxs map, described below.
     */
    VariablesMapByParticle m_variablesmap;

    /**
     * Internal map.
     * The key corresponds to the global bin index in the 2D (theta,p) grid.
     * The mapped value is a list of variables used for the MVA training
     * for a given particle in a given 2D bin.
     * NB: the order of the variables in the mapped vector matches the one used in the training by construction!
    */
    VariablesMapByBinIdxs m_variablesmap_bybinidxs;

    /**
     * To be toggled on if input variables have been transformed
     * (i.e., if storeVarsTransfoSettings() was called when creating the payload).
     */
    bool m_do_varstransform = false;

    /**
     * This map contains the 2D (theta,p) grid histograms describing - for each particle hypothesis - the categories for which PDFs are defined.
     * The key corresponds to the particle hypothesis' signed pdgId.
     * The mapped value is the 2D (theta,p) bin grid as a histogram.
     */
    BinsHistoByParticle m_binshisto;

    /**
     * Internal map.
     * The key corresponds to the particle hypothesis' signed pdgId.
     * The mapped value is a PdfsMapByBinIdxs map, described below.
     */
    PdfsMapByParticle m_pdfsmap;

    /**
     * Internal map.
     * The key corresponds to the global bin index in the 2D (theta,p) grid.
     * The mapped value is a PdfsByVariable map, described below.
     */
    PdfsMapByBinIdxs m_pdfsmap_bybinidxs;

    /**
     * Internal map.
     * The key corresponds to an observable of interest (E/p, E, shower shape...).
     * The mapped value is a TF1 representing the normalised PDF for that variable, for a given particle in a given 2D bin.
     */
    PdfsByVariable m_pdfs_byvariable;

    /**
     * Internal map.
     * The key corresponds to the particle hypothesis' signed pdgId.
     * The mapped value is a VTSMapByBinIdxs map, described below.
     */
    VTSMapByParticle m_vtsmap;

    /** This map contains the actual PDFs, for each particle hypothesis and 2D bin indexes.
    The key corresponds to the particle hypothesis' signed pdgId.
    The mapped value is the "internal" map described above.
    */
    PdfsMapByParticle m_pdfsmap;

    ClassDef(ECLChargedPidPDFs, 1); /**< ClassDef */
  };
} // end namespace Belle2
