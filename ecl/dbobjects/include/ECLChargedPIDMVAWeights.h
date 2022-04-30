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
#include <TF1.h>
#include <TParameter.h>
#include <TFile.h>

//BOOST
#include <boost/algorithm/string/predicate.hpp>

// std libraries
#include <cmath>
#include <unordered_map>
#include <tuple>

#include <iostream>


namespace Belle2 {

  /** Class to contain payload of everything needed for MVA based charged particle identification.
  * For each bin of (theta, p, charge), this includes:
  *  - MVA weightfiles for multiclass BDT.
  *  - TF1 p.d.fs for each charged particle hypothesis for each bdt output variable.
  *  - (Optional) TH1F for each charged particle hypothesis for each bdt output variable for gaussianisation.
  *  - (Optional) vector of floats (flattened square matrix) for potential linear decorrelation of the gaussian transformed bdt response variables.
  *
  */

  class ECLChargedPIDMVAWeights : public TObject {

  public:
    /**
    * Default constructor, necessary for ROOT to stream the object.
    */
    ECLChargedPIDMVAWeights():
      m_energy_unit("energyUnit", Unit::GeV),
      m_ang_unit("angularUnit", Unit::rad),
      m_log_transform_offset("logTransformOffset", 1e-15)
    {};

    /**
     * Destructor.
     */
    ~ECLChargedPIDMVAWeights() {};


    enum class BDTResponseTransformMode : unsigned int {
      /** log transform the bdt responses. Default mode */
      c_LogTransform = 0,
      /** log transform the bdt responses. Take the likelihood from only the bdt response for the hypothesis */
      c_LogTransformSingle = 1,
      /** Gaussian transform of the log transformed bdt response. */
      c_GaussianTransform = 2,
      /** Decorrelation transform of the gaussian transformed bdt responses. */
      c_DecorrelationTransform = 3
    };


    /**
     * Set the energy unit to ensure consistency w/ the one used to define the bins grid.
     * @param unit: the energy unit.
     */
    void setEnergyUnit(const float& unit)
    {
      m_energy_unit.SetVal(unit);
    }


    /**
     * Set the angular unit to ensure consistency w/ the one used to define the bins grid.
     * @param unit: the angular unit.
     */
    void setAngularUnit(const float& unit)
    {
      m_ang_unit.SetVal(unit);
    }

    /**
     * Set the offset used in the log transformation to be consistent with the offset used when generating the p.d.f.s
     */
    void setlogTransformOffset(const float& offset)
    {
      m_log_transform_offset.SetVal(offset);
    }

    /**
     * Set the 3D (clusterTheta, p, charge) grid representing the categories for which weightfiles are defined.
     * @param h the 3D histogram.
    */
    void setWeightCategories(TH3F* h)
    {
      m_categories = h;
    }


    /**
     * checks if the input index is the same as that returned for the given (theta, p, charge) triplet
     */
    void checkIndexConsistency(const unsigned int idx, const float theta, const float p, const float charge) const
    {
      B2FATAL("(theta = " << theta << ", p = " << p << ", charge = " << charge << ")");
      auto h_idx = getLinearisedBinIndex(theta, p, charge);
      if (idx != h_idx) {
        B2FATAL("index in input vector:\n" << idx << "\ndoes not correspond to:\n" << h_idx <<
                "\n, i.e. the linearised index of the 3D bin centered in (clusterTheta, p, charge) = (" << theta << ", " << p << ", " << charge <<
                ")\nPlease check how the input vector is being filled.");
      }
    }

    /**
     * store the MVA weight files (one for each category) into the payload.
     *
     * @param filepaths a vector of xml (root) file paths for all (theta, p, charge) categories.
     * @param variables a vector of vectors filled with std strings. For each index gives the list of classifier variables.
              Note that these should be stored in the full version without aliasing! Otherwise the aliases will need to be defined
              to match during runtime.
     * @param transformations a vector of BDTResponseTransformMode for all (theta, p, charge) categories.
     * @param pdfs a vector of vectors of unsigned maps with TF1 pdfs for all charged hypothesis
              for all bdt response values for all (theta, p, charge) categories.
     * @param cdfs a vector of vectors of unsigned maps with TH1F histograms for all charged hypothesis
              for all bdt response values for all (theta, p, charge) categories.
     * @param decorrelationMatrices a vector of unsigned maps with TMatrix for all charged hypothesis
              for all (theta, p, charge) categories.
     * @param categoryBinCentres a vector of tuple<float, float, float> representing the (theta, p, charge) bin centres.
     *        Used to check consistency of the xml vector indexing w/ the linearised TH3 category map.
     */
    void storeMVAWeights(std::vector<std::string>& filepaths,
                         std::vector<std::vector<std::string>>& variables,
                         std::vector<BDTResponseTransformMode>& transformations,
                         std::vector<std::vector<std::unordered_map<unsigned int, TF1>>>& pdfs,
                         std::unordered_map<unsigned int, std::vector<std::unordered_map<unsigned int, TH1F>>>& cdfs,
                         std::unordered_map<unsigned int, std::unordered_map<unsigned int, std::vector<float>>>& decorrelationMatrices,
                         std::vector<std::tuple<double, double, double>>& categoryBinCentres)
    {
      for (unsigned int idx = 0; idx < filepaths.size(); idx++) {
        // loose check for consistency
        auto bin_centers = categoryBinCentres.at(idx);
        auto t_center = std::get<0>(bin_centers);
        auto p_center = std::get<1>(bin_centers);
        auto c_center = std::get<2>(bin_centers);

        Belle2::MVA::Weightfile weightfile;
        if (boost::ends_with(filepaths[idx], ".root")) {
          weightfile = Belle2::MVA::Weightfile::loadFromROOTFile(filepaths[idx]);
        } else  if (boost::ends_with(filepaths[idx], ".xml")) {
          weightfile = Belle2::MVA::Weightfile::loadFromXMLFile(filepaths[idx]);
        } else {
          B2WARNING("Unkown file extension for file: " << filepaths[idx] << ", fallback to xml...");
          weightfile = Belle2::MVA::Weightfile::loadFromXMLFile(filepaths[idx]);
        }

        // Serialize the MVA::Weightfile object into a string for storage in the database,
        // otherwise there are issues w/ dictionary generation for the payload class...
        std::stringstream ss;
        Belle2::MVA::Weightfile::saveToStream(weightfile, ss);
        m_weights.push_back(ss.str());

        m_variables.push_back(variables[idx]);
        m_bdtResponseTransformModes.push_back(transformations[idx]);
        m_pdfs.push_back(pdfs[idx]);

        // These are optional based on which transformation mode is used.
        if (transformations[idx] == BDTResponseTransformMode::c_GaussianTransform) {
          m_cdfs[idx] = cdfs[idx];
        } else if (transformations[idx] == BDTResponseTransformMode::c_DecorrelationTransform) {
          m_cdfs[idx] = cdfs[idx];
          m_decorrelationMatrices[idx] = decorrelationMatrices[idx];
        }
      }
    }

    const std::vector<std::string>* getMVAWeightStrings() const
    {
      return &m_weights;
    }

    /**
    * returns bool whether or not the given p, theta, charge values are within the phasespace covered by the trainings in the weightfile
    * @param theta: theta of the cluster [rad].
    * @param p: momentum of the track [GeV].
    * @param charge: charge of the track.
    */
    bool isPhasespaceCovered(const float theta, const float p, const float charge) const
    {
      if (!m_categories) {
        B2FATAL("No (clusterTheta, p, charge) TH3 grid was found in the ECLChargedPIDMVA DB payload. This should not happen! Abort...");
      }

      const float ttheta = theta  / m_ang_unit.GetVal();
      const float pp = p / m_energy_unit.GetVal();

      if (ttheta < m_categories->GetXaxis()->GetBinLowEdge(1)) return false;
      if (ttheta >= m_categories->GetXaxis()->GetBinLowEdge(m_categories->GetXaxis()->GetNbins() + 1)) return false;

      if (pp < m_categories->GetYaxis()->GetBinLowEdge(1)) return false;
      if (pp >= m_categories->GetYaxis()->GetBinLowEdge(m_categories->GetYaxis()->GetNbins() + 1)) return false;

      if (charge < m_categories->GetZaxis()->GetBinLowEdge(1)) return false;
      if (charge >= m_categories->GetZaxis()->GetBinLowEdge(m_categories->GetZaxis()->GetNbins() + 1)) return false;

      return true;
    }

    /**
    * returns the flattened 1D bin index of the 3D binning.
    * @param theta: theta of the cluster [rad].
    * @param p: momentum of the track [GeV].
    * @param charge: charge of the track.
    */
    unsigned int getLinearisedBinIndex(const float theta, const float p, const float charge) const
    {
      if (!m_categories) {
        B2FATAL("No (clusterTheta, p, charge) TH3 grid was found in the ECLChargedPIDMVA DB payload. This should not happen! Abort...");
      }
      // alternatively set these to be just inside the first or last bin.
      if (!isPhasespaceCovered(theta, p, charge)) {
        B2FATAL("Attempting to get bin index for event with (theta = " << theta << ", p = " << p << ", charge = " << charge <<
                ") outside the covered phasespace. This should not happen! Abort...");
      }
      // This is different to the root FindBin index as that includes under and overflow bins in each dimension.
      const unsigned int nTheta  = m_categories->GetXaxis()->GetNbins();
      const unsigned int nP      = m_categories->GetYaxis()->GetNbins();

      const unsigned int iTheta  = m_categories->GetXaxis()->FindBin(theta / m_ang_unit.GetVal()) - 1;
      const unsigned int iP      = m_categories->GetYaxis()->FindBin(p / m_energy_unit.GetVal()) - 1;
      const unsigned int iCharge = m_categories->GetZaxis()->FindBin(charge) - 1;


      return iTheta + nTheta * (iP + nP * iCharge);
    }

    /**
     * get the log transform offset
    */
    const float getLogTransformOffset() const
    {
      return m_log_transform_offset.GetVal();
    }


    /**
    * gets the cdf for the hypothesis pdg for a given response value for a given phase space region.
    */
    const TH1F* getCDF(const int hypoPDG, const unsigned int iBDTResponse, const unsigned int linearBinIndex) const
    {
      return &m_cdfs.at(linearBinIndex).at(iBDTResponse).at(hypoPDG);
    }

    /**
    * gets the cdf for the hypothesis pdg for a given response value for a given phase space region.
    */
    const TH1F* getCDF(const int hypoPDG, const unsigned int iBDTResponse, const float theta, const float p, const float charge) const
    {
      unsigned int linearBinIndex = getLinearisedBinIndex(theta, p, charge);
      return getCDF(hypoPDG, iBDTResponse, linearBinIndex);
    }


    /**
    * gets the pdf for the hypothesis pdg for a given response value for a given phase space region.
    */
    const TF1* getPDF(const int hypoPDG, const unsigned int iBDTResponse, const unsigned int linearBinIndex) const
    {
      return &m_pdfs.at(linearBinIndex).at(iBDTResponse).at(hypoPDG);
    }

    /**
    * gets the pdf for the hypothesis pdg for a given response value for a given phase space region.
    */
    const TF1* getPDF(const int hypoPDG, const unsigned int iBDTResponse, const float theta, const float p, const float charge) const
    {
      unsigned int linearBinIndex = getLinearisedBinIndex(theta, p, charge);
      return getPDF(hypoPDG, iBDTResponse, linearBinIndex);
    }

    /**
    * gets the decorrelation matrix for a given response value for a given phase space region.
    */
    const std::vector<float>* getDecorrelationMatrix(const int hypoPDG, const unsigned int linearBinIndex) const
    {
      return &m_decorrelationMatrices.at(linearBinIndex).at(hypoPDG);
    }

    /**
    * gets the decorrelation matrix for the hypothesis pdg for a given phase space region.
    */
    const std::vector<float>* getDecorrelationMatrix(const int hypoPDG, const float theta, const float p, const float charge)
    {
      unsigned int linearBinIndex = getLinearisedBinIndex(theta, p, charge);
      return getDecorrelationMatrix(hypoPDG, linearBinIndex);
    }

    const BDTResponseTransformMode* getTransformMode(const unsigned int linearBinIndex)  const
    {
      return &m_bdtResponseTransformModes.at(linearBinIndex);
    }

    const BDTResponseTransformMode* getTransformMode(const float theta, const float p, const float charge)  const
    {
      unsigned int linearBinIndex = getLinearisedBinIndex(theta, p, charge);
      return getTransformMode(linearBinIndex);
    }


  private:
    TParameter<float> m_energy_unit; /**< The energy unit used for defining the bins grid. */
    TParameter<float> m_ang_unit;    /**< The angular unit used for defining the bins grid. */
    TParameter<float> m_log_transform_offset; /**< Small offset to avoid bdt response values of 1.0 being log transformed to NaN. */

    /**
     * A 3D (theta, p, charge) histogram whose bins represent the categories for which the training is performed .
      * It is used to lookup the correct file in the payload, given a reconstructed triplet (theta, p, charge).
     */
    TH3F* m_categories = nullptr;

    /**
     * Stores the weightfiles for all the (theta, p, charge) categories.
     */
    std::vector<std::string> m_weights;

    /**
     * Stores the classifier variables for all the (theta, p, charge) categories.
     */
    std::vector<std::vector<std::string>> m_variables;

    /**
     * Stores which transformation mode to apply to the bdt responses.
     */
    std::vector<BDTResponseTransformMode> m_bdtResponseTransformModes;

    /**
     * Stores whether 5 (no deuteron) or 6 BDT response are expected for the particular bin;
     */
    std::vector<bool> m_separate_deuteron_response;

    /**
     * A vector of vectors of unodered maps. The outer vector corresponds to the phasespace region. The inner vector to the N return values of the BDT.
     * The unordered map maps the hypothesis pdg values to their matching TF1 pdfs from which the liklihood will be taken.
     */
    std::vector < std::vector<std::unordered_map<unsigned int, TF1>>> m_pdfs;


    /**
     * An unordered map of vectors of unodered maps. The outer map corresponds to the phasespace region. The vector to the N return values of the BDT.
     * The inner unordered map maps the hypothesis pdg values to their matching TH1F cdfs which can be used for a gaussianisation.
     */
    std::unordered_map < unsigned int, std::vector<std::unordered_map<unsigned int, TH1F>>> m_cdfs;

    /**
     * An unordered map of unodered maps. The outer map corresponds to the phasespace region.
     * The inner unordered map maps the hypothesis pdg values to their matching matrix which can be used to perform a decorrelation transformation on the gaussian transformed variables.
     */
    std::unordered_map < unsigned int, std::unordered_map<unsigned int, std::vector<float>>> m_decorrelationMatrices;


    /**< 1: first class implementation. */
    ClassDef(ECLChargedPIDMVAWeights, 1);

  }; // class ECLChargedPIDMVAWeights
} // Belle 2 Namespace