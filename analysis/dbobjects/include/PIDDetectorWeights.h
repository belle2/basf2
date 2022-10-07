/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <boost/algorithm/string.hpp>
#include <set>

#include <TObject.h>
#include <ROOT/RDataFrame.hxx>
#include <ROOT/RCsvDS.hxx>


namespace Belle2 {

  /**
   * Class for handling the PID weights per detector, used to calculate the track helix isolation score per particle.
   */
  class PIDDetectorWeights : public TObject {

  public:

    /**
     * Nested class acting as a container the per-detector weights.
     * Conceptually, an instance of this class is associated to one std charged hypothesis.
     */
    class WeightsTable {
    public:

      WeightsTable() :
        m_isEmpty(false),
        m_nPBins(0),
        m_nThetaBins(0)
      {}; /**< Default constructor */

      bool m_isEmpty; /**< * Flag to indicate whether the internal containers wwere filled. */

      std::set<double> m_pBinEdges; /**< Set of p bins edges. */
      std::set<double> m_thetaBinEdges; /**< Set of theta bins edges. */

      int m_nPBins; /**< Number of p bins. */
      int m_nThetaBins; /**< Number of theta bins. */

      /**
       * Map the linearised index of the (p, theta) bin indexes to the row index in the (filtered) table.
       * Used for fast lookup in the event loop.
      */
      std::unordered_map<double, unsigned int> m_linBinIdxsToRowIdxs;
      /**
       * Map each detector to its vector of weights.
       * By construction, the size of each vector is equal to the number of rows in the (filtered) table.
       */
      std::map<std::string, std::vector<double>> m_weightsPerDet;

    private:

      ClassDef(WeightsTable, 1); /**< Needed for sub-class schema evolution. */
    };

    /**
     * Default constructor, necessary for ROOT to stream the object.
     */
    PIDDetectorWeights() {};

    /**
     * Destructor.
     */
    ~PIDDetectorWeights() {};

    /**
     * Constructor from CSV file of weights.
     * NB: please ensure all numeric types are stored as double in the CSV!
     * @param  weightsCSVFileName the path to the CSV file conatining the detector weights per std charged particle hypothesis,
     * in bins of p and theta.
    */
    PIDDetectorWeights(const std::string& weightsCSVFileName)
    {
      m_weightsRDataFrame = ROOT::RDF::MakeCsvDataFrame(weightsCSVFileName);
      fillWeightsTablePerHypoFromRDF();
    };

    /**
     * Constructor from ROOT file w/ TTree of weights.
     * @param treeName the name of the TTree with detector weights per std charged particle hypothesis, in bins of p and theta.
     * @param weightsROOTFileName the path to the ROOT file containing the TTree.
    */
    PIDDetectorWeights(const std::string& treeName, const std::string& weightsROOTFileName) :
      m_weightsRDataFrame(treeName, weightsROOTFileName)
    {
      fillWeightsTablePerHypoFromRDF();
    };

    /**
     * Get the RDataFrame of detector weights.
     * To be used for testing/debugging only when creating the payload.
     */
    ROOT::RDataFrame getWeightsRDF() const
    {
      return m_weightsRDataFrame;
    };

    /**
     * Lookup the weight from the internal map structures.
     * @param hypo the input std charged particle.
     * @param det the input PID detector.
     * @param p the particle momentum in [GeV/c].
     * @param theta the particle polar angle in [rad].
     */
    double getWeight(Const::ChargedStable hypo, Const::EDetector det, double p, double theta) const;

  private:

    /**
     * Map containing a WeightsTable object per particle hypo.
     * Use to lookup the index corresponding to a (p, theta) pair.
     */
    std::map<int, WeightsTable> m_weightsTablePerHypo;

    /**
     * The RDataFrame containing the detector weights per particle hypo, per phase space bin.
     * Using an RDataFrame is convenient when creating the payload from an input CSV file.
     * However, an RDataFrame cannot be streamed in the persistent ROOT file, thus we must prevent this from happening.
     */
    ROOT::RDataFrame m_weightsRDataFrame = ROOT::RDataFrame(1); //!

    /**
     * The names of the per-detector weight columns in the RDataFrame
     */
    std::map<std::string, std::string> m_weightNames = {
      {Const::parseDetectors(Const::CDC), "ablat_s_CDC"},
      {Const::parseDetectors(Const::TOP), "ablat_s_TOP"},
      {Const::parseDetectors(Const::ARICH), "ablat_s_ARICH"},
      {Const::parseDetectors(Const::ECL), "ablat_s_ECL"},
      {Const::parseDetectors(Const::KLM), "ablat_s_KLM"}
    };

    /**
     * Fill the internal weights container class per particle hypo, based on the content of the RDataFrame.
     */
    void fillWeightsTablePerHypoFromRDF();

    ClassDef(PIDDetectorWeights, 4); /**< ClassDef as this is a TObject */

  };

} // Belle2 namespace
