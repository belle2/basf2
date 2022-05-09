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

namespace Belle2 {

  /**
   * Stores all required information for the ECLChargedPIDMVA for a phasespace category - (clusterTheta, p, charge) region.
   * This includes:
   *  - MVA weightfiles for multiclass MVA.
   *  - TF1 p.d.fs for each charged particle hypothesis for each mva output variable.
   *  - MVAResponseTransform mode detailing which transformations will be applied to the MVA response.
   *  - unordered_map mapping a particle hypothesis to the output index of a MVA.
   *  - (Optional) TH1F for each charged particle hypothesis for each mva output variable for gaussianisation.
   *  - (Optional) vector of floats (flattened square matrix) for potential linear decorrelation of the gaussian transformed mva response variables.
   */

  class ECLChargedPIDPhasespaceCategory : public TObject {

  public:

    enum class MVAResponseTransformMode : unsigned int {
      /** log transform the mva responses. And take the likelihood as the product of likelihoods from all mva responses. */
      c_LogTransform = 0,
      /** log transform the mva responses. Take the likelihood from only the mva response for the hypothesis. */
      c_LogTransformSingle = 1,
      /** Gaussian transform of the log transformed mva response. */
      c_GaussianTransform = 2,
      /** Decorrelation transform of the gaussian transformed mva responses. */
      c_DecorrelationTransform = 3
    };

    /**
    * Default constructor, necessary for ROOT to stream the object.
    */
    ECLChargedPIDPhasespaceCategory() :
      m_log_transform_offset("logTransformOffset", 1e-15)
    {};

    /**
    * Useful constructor.
    * @param weightfilePath path to the MVA weightfile for this phasespace category.
    * @param mvaResponeTransformMode mva response transform mode booked for this phasespace.
    * @param pdfs vector of unordered_map mapping hypothesis to pdfs for each mva response.
    * @param mvaIndexForHypothesis unordered_map mapping hypothesis to index of mva response. Useful if we exclude a class from the MVA training
    *        but want to use the likelihood of a different particle. For example using the proton pdf for a deuteron.
    */
    ECLChargedPIDPhasespaceCategory(const std::string weightfilePath,
                                    const MVAResponseTransformMode& mvaResponeTransformMode,
                                    const std::vector<std::unordered_map<unsigned int, TF1>>& pdfs,
                                    const std::unordered_map<unsigned int, unsigned int>& mvaIndexForHypothesis) :

      m_log_transform_offset("logTransformOffset", 1e-15)

    {
      // Load and serialize the MVA::Weightfile object into a string for storage in the database,
      // otherwise there are issues w/ dictionary generation for the payload class...
      Belle2::MVA::Weightfile weightfile;
      if (boost::ends_with(weightfilePath, ".root")) {
        weightfile = Belle2::MVA::Weightfile::loadFromROOTFile(weightfilePath);
      } else  if (boost::ends_with(weightfilePath, ".xml")) {
        weightfile = Belle2::MVA::Weightfile::loadFromXMLFile(weightfilePath);
      } else {
        B2WARNING("Unkown file extension for file: " << weightfilePath << ", fallback to xml...");
        weightfile = Belle2::MVA::Weightfile::loadFromXMLFile(weightfilePath);
      }
      std::stringstream ss;
      Belle2::MVA::Weightfile::saveToStream(weightfile, ss);

      // store
      m_weight = ss.str();
      m_mvaResponseTransformMode = mvaResponeTransformMode;
      m_pdfs = pdfs;
      m_mvaIndexForHypothesis = mvaIndexForHypothesis;
    }

    /**
     * Destructor.
     */
    ~ECLChargedPIDPhasespaceCategory() {};

    /**
     * getter for serialised weightfile.
     */
    const std::string getSerialisedWeight() const {return m_weight;}

    /**
     * getter for the MVA transform mode.
     */
    MVAResponseTransformMode getTransformMode() const {return m_mvaResponseTransformMode;}

    /**
     * getter for pdfs.
     * @param iMVAResponse index of MVA response.
     * @param hypoPDG, hypothesis pdg.
     */
    const TF1* getPDF(const unsigned int iMVAResponse, const unsigned int hypoPDG) const
    {
      return &m_pdfs.at(iMVAResponse).at(hypoPDG);
    }

    /**
     * gets the cdf for the hypothesis pdg for a given response value.
     * @param iMVAResponse index of MVA response.
     * @param hypoPDG, hypothesis pdg.
     */
    const TH1F* getCDF(const unsigned int iMVAResponse, const int hypoPDG) const
    {
      return &m_cdfs.at(iMVAResponse).at(hypoPDG);
    }

    /**
     * gets the decorrelation matrix for a given particle hypothesis.
     * @param hypoPDG, hypothesis pdg.
     */
    const std::vector<float>* getDecorrelationMatrix(const int hypoPDG) const
    {
      return &m_decorrelationMatrices.at(hypoPDG);
    }


    /**
     * set the cdfs.
     * @param vector of map of cdfs to be stored in the payload.
     */
    void setCDFs(std::vector<std::unordered_map<unsigned int, TH1F>> cdfs) {m_cdfs = cdfs;}

    /**
     * set the decorrelation matrices.
     * @param decorrelationMatrices map of decorrelation matrices to be stored in the payload.
     */
    void setDecorrelationMatrixMap(std::unordered_map<unsigned int, std::vector<float>> decorrelationMatrices)
    {
      m_decorrelationMatrices = decorrelationMatrices;
    }

    /**
     * Set the offset used in the log transformation to be consistent with the offset used when generating the p.d.f.s
     */
    void setlogTransformOffset(const float& offset)
    {
      m_log_transform_offset.SetVal(offset);
    }

    /**
     * get the log transform offset
     */
    float getLogTransformOffset() const
    {
      return m_log_transform_offset.GetVal();
    }

    /**
     * maps a charged stable pdg code to an index of the MVA response.
     * In general this is a one-to-one mapping however in cases where we do not include all six
       stable charged particles in the MVA training we may have a many-to-one mapping.
     * For example if we take the proton response value also for deuterons.
     */
    unsigned int getMVAIndexForHypothesis(const unsigned int hypoPDG) const
    {
      return m_mvaIndexForHypothesis.at(hypoPDG);
    }

  private:

    TParameter<float> m_log_transform_offset; /**< Small offset to avoid mva response values of 1.0 being log transformed to NaN. */

    /**
     * Serialsed MVA weightfile.
     */
    std::string m_weight;

    /**
     * Stores which transformation mode to apply to the mva responses.
     */
    MVAResponseTransformMode m_mvaResponseTransformMode;


    /**
     * A vector of unodered maps. The vector corresponds to the return values of the MVA, one for each class correspondonding to charged stable particles considered by the MVA. In general this is the full six charged stable particles {e, mu, pi, K, p, d}.
     * The unordered map maps the hypothesis pdg values to their matching TF1 pdfs from which the liklihood will be taken.
     */
    std::vector<std::unordered_map<unsigned int, TF1>> m_pdfs;

    /**
     * unordered map of abs(pdg_code) for the 6 charged stable hypotheses to index of the MVA response vector.
     * needed if we do not train with all 6 species to map several to the same MVA response value.
     */
    std::unordered_map<unsigned int, unsigned int> m_mvaIndexForHypothesis;

    /**
     * CDFs for each mva return value for each hypothesis.
     * The N vector elements correspond to the N MVA return values.
     * The unordered map maps the hypothesis pdg values to their matching TH1F cdfs which can be used for a gaussianisation.
     */
    std::vector<std::unordered_map<unsigned int, TH1F>> m_cdfs;


    /**
     * Decorrelation matrices. To be used (optionally) afer gaussianisation.
     * The unordered map maps the hypothesis pdg values to their matching linearised decorrelation matrix.
     */
    std::unordered_map<unsigned int, std::vector<float>> m_decorrelationMatrices;

    /**< 1: first class implementation. */
    ClassDef(ECLChargedPIDPhasespaceCategory, 1);
  };

  /** Class to contain payload of everything needed for MVA based charged particle identification.
    * - TH3 object specifying the (clusterTheta, p, charge) boundaries that define the categories (regions) under consideration,
    * - Vector of ECLChargedPIDPhasespaceCategory objects which contain specific settings for each category.
    */
  class ECLChargedPIDMVAWeights : public TObject {
  public:
    /**
    * Default constructor, necessary for ROOT to stream the object.
    */
    ECLChargedPIDMVAWeights():
      m_energy_unit("energyUnit", Unit::GeV),
      m_ang_unit("angularUnit", Unit::rad)
    {};

    /**
     * Destructor.
     */
    ~ECLChargedPIDMVAWeights() {};

    /**
     * Set the energy unit to ensure consistency w/ the one used to define the phasespace category grid.
     * @param unit: the energy unit.
     */
    void setEnergyUnit(const float& unit)
    {
      m_energy_unit.SetVal(unit);
    }


    /**
     * Set the angular unit to ensure consistency w/ the one used to define the phasespace category grid.
     * @param unit: the angular unit.
     */
    void setAngularUnit(const float& unit)
    {
      m_ang_unit.SetVal(unit);
    }


    /**
     * Set the 3D (clusterTheta, p, charge) grid representing the categories for which weightfiles are defined.
     * @param h the 3D histogram in (clusterTheta, p, charge).
     * A multiclass MVA is trained for each phases-space region defined by the bin boundaries.
    */
    void setWeightCategories(TH3F* h)
    {
      m_categories = h;
    }


    /**
     * checks if the input index is the same as that returned for the given (clusterTheta, p, charge) triplet
     */
    void checkIndexConsistency(const unsigned int idx, const float clusterTheta, const float p, const float charge) const
    {
      B2FATAL("(clusterTheta = " << clusterTheta << ", p = " << p << ", charge = " << charge << ")");
      auto h_idx = getLinearisedCategoryIndex(clusterTheta, p, charge);
      if (idx != h_idx) {
        B2FATAL("index in input vector:\n" << idx << "\ndoes not correspond to:\n" << h_idx <<
                "\n, i.e. the linearised index of the 3D category centered in (clusterTheta, p, charge) = (" << clusterTheta << ", " << p << ", " <<
                charge <<
                ")\nPlease check how the input vector is being filled.");
      }
    }

    /**
     * store the ECLChargedPIDPhasespaceCategory objects into the payload.
     * @param phasespaceCategories a vector of ECLChargedPIDPhasespaceCategory objects, one per phasespace region.
              Each object contains all the data required to process tracks in that phasespace.
     */
    void storeMVAWeights(std::vector<ECLChargedPIDPhasespaceCategory>& phasespaceCategories)
    {
      m_phasespaceCategories = phasespaceCategories;
    }

    /**
     * returns number of phasespace categories.
     */
    unsigned int nCategories() const {return m_phasespaceCategories.size();}

    /**
     * returns the ith ECLChargedPIDPhasespaceCategory.
     * @param idx, index of ECLChargedPIDPhasespaceCategory.
     */
    const ECLChargedPIDPhasespaceCategory* getPhasespaceCategory(const unsigned int idx)  const {return &m_phasespaceCategories.at(idx);}

    /**
     * returns the vector of phasespaceCategories.
     */
    const std::vector<ECLChargedPIDPhasespaceCategory>* getPhasespaceCategories() const {return &m_phasespaceCategories;}

    /**
    * returns bool whether or not the given p, clusterTheta, charge values are within the phasespace covered by the trainings in the weightfile
    * @param clusterTheta: clusterTheta of the cluster [rad].
    * @param p: momentum of the track [GeV].
    * @param charge: charge of the track.
    */
    bool isPhasespaceCovered(const float clusterTheta, const float p, const float charge) const
    {
      if (!m_categories) {
        B2FATAL("No (clusterTheta, p, charge) TH3 grid was found in the ECLChargedPIDMVA DB payload. This should not happen! Abort...");
      }

      const float ttheta = clusterTheta  / m_ang_unit.GetVal();
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
    * returns the flattened 1D index of the 3D phasespace category grid.
    * @param clusterTheta: clusterTheta of the cluster [rad].
    * @param p: momentum of the track [GeV].
    * @param charge: charge of the track.
    */
    unsigned int getLinearisedCategoryIndex(const float clusterTheta, const float p, const float charge) const
    {
      if (!m_categories) {
        B2FATAL("No (clusterTheta, p, charge) TH3 grid was found in the ECLChargedPIDMVA DB payload. This should not happen! Abort...");
      }
      // alternatively set these to be just inside the nearest category.
      if (!isPhasespaceCovered(clusterTheta, p, charge)) {
        B2FATAL("Attempting to get bin index for event with (clusterTheta = " << clusterTheta << ", p = " << p << ", charge = " << charge <<
                ") outside the covered phasespace. This should not happen! Abort...");
      }
      // This is different to the root FindBin index as that includes under and overflow bins in each dimension.
      const unsigned int nTheta  = m_categories->GetXaxis()->GetNbins();
      const unsigned int nP      = m_categories->GetYaxis()->GetNbins();

      const unsigned int iTheta  = m_categories->GetXaxis()->FindBin(clusterTheta / m_ang_unit.GetVal()) - 1;
      const unsigned int iP      = m_categories->GetYaxis()->FindBin(p / m_energy_unit.GetVal()) - 1;
      const unsigned int iCharge = m_categories->GetZaxis()->FindBin(charge) - 1;


      return iTheta + nTheta * (iP + nP * iCharge);
    }


  private:
    TParameter<float> m_energy_unit; /**< The energy unit used for defining the 3D (clusterTheta, p, charge) category grid. */
    TParameter<float> m_ang_unit;    /**< The angular unit used for defining the 3D (clusterTheta, p, charge) category grid. */

    /**
     * A 3D (clusterTheta, p, charge) histogram whose bins define the boundaries of the categories for which the training is performed.
      * It is used to lookup the correct file in the payload, given a reconstructed triplet (clusterTheta, p, charge).
     */
    TH3F* m_categories = nullptr;

    /**
     * Stores the ECLChargedPIDPhasespaceCategory object for all the (clusterTheta, p, charge) categories.
     */
    std::vector<ECLChargedPIDPhasespaceCategory> m_phasespaceCategories;


    /**< 1: first class implementation. */
    ClassDef(ECLChargedPIDMVAWeights, 1);

  }; // class ECLChargedPIDMVAWeights
} // Belle 2 Namespace