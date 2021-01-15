#ifndef GRLNEURO_H
#define GRLNEURO_H

#include <trg/grl/dataobjects/GRLMLP.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
//#include <trg/cdc/dbobjects/CDCTriggerNeuroConfig.h>
//#include <framework/dataobjects/BinnedEventT0.h>

namespace Belle2 {

  /** Class to represent the GRL Neuro.
   *
   * The Neurotrigger consists of one or several Multi Layer Perceptrons.
   * The input values are calculated from ECLTRG cluster and a 2D track estimate.
   * The output is a scaled estimate of the judgement.
   *
   * @sa GRLNeuro Modules:
   * @sa GRLTrainerModule for preparing training data and training,
   * @sa GRLNeuro for loading trained networks and using them.
   */
  class GRLNeuro {
  public:
    /** Struct to keep neurotrigger parameters.
     * Contains all information that is needed to initialize several
     * expert MLPs (not including values determined during training).
     */
    struct Parameters {
      /** Number of networks.
       * For network specific parameters you can give either a list with
       * values for each network, or a single value that will be used for all.
       * The ranges are also valid if nPhi * nPt * nTheta * nPattern = nMLPs.
       */
      unsigned nMLP;
      /** Number of nodes in each hidden layer for all networks
       * or factor to multiply with number of inputs.
       * The number of layers is derived from the shape.
       */
      std::vector<std::vector<float>> nHidden;
      /** train result as output */
      bool targetresult = true;
      /** If true, multiply nHidden with number of input nodes. */
      bool multiplyHidden = false;
      /** Output scale for all networks. */
      std::vector<std::vector<float>> outputScale = {{ -1., 1.}};

      /** Number of CDC sectors. */
      unsigned n_cdc_sector;
      std::vector<float> i_cdc_sector;
      /** Number of ECL sectors. */
      unsigned n_ecl_sector;
      std::vector<float> i_ecl_sector;
    };

    /** Default constructor. */
    GRLNeuro() {}

    /** Default destructor. */
    virtual ~GRLNeuro() {}

    /** Set parameters and get some network independent parameters. */
    void initialize(const Parameters& p);

    /** Save MLPs to file.
     * @param filename name of the TFile to write to
     * @param arrayname name of the TObjArray holding the MLPs in the file
     */
    void save(const std::string& filename, const std::string& arrayname = "MLPs");
    /** Load MLPs from file.
     * @param filename name of the TFile to read from
     * @param arrayname name of the TObjArray holding the MLPs in the file
     * @return true if the MLPs were loaded correctly
     */
    bool load(const std::string& filename, const std::string& arrayname = "MLPs");

    /** Loads parameters from the geometry and precalculates some constants
     * that will be needed. */
    //void setConstants();

    /** set fixed point precision */
    void setPrecision(std::vector<unsigned> precision) { m_precision = precision; }

    /** set the hit collection and event time to required
     * and store the hit collection name */
    //void initializeCollections(std::string hitCollectionName, std::string eventTimeName, std::string et_option);

    /** return reference to a neural network */
    GRLMLP& operator[](unsigned index) { return m_MLPs[index]; }
    /** return const reference to a neural network */
    const GRLMLP& operator[](unsigned index) const { return m_MLPs[index]; }

    /** return number of neural networks */
    unsigned nSectors() const { return m_MLPs.size(); }

    /** add an MLP to the list of networks */
    void addMLP(const GRLMLP& newMLP) { m_MLPs.push_back(newMLP); }

    /** Run an expert MLP.
     * @param isector index of the MLP
     * @param input vector of input values
     * @return unscaled output values (z vertex in cm and/or theta in radian) */
    std::vector<float> runMLP(unsigned isector, std::vector<float> input);

    /** Run an expert MLP with fixed point arithmetic. */
    std::vector<float> runMLPFix(unsigned isector, std::vector<float> input);

  private:
    /** List of networks */
    std::vector<GRLMLP> m_MLPs = {};
    /** Fixed point precision in bit after radix point.
     *  8 values:
     *  - 2D track parameters: omega, phi
     *  - geometrical values derived from track: crossing angle, reference wire ID
     *  - scale factor: radian to wire ID
     *  - MLP values: nodes, weights, activation function LUT input (LUT output = nodes)
     */
    std::vector<unsigned> m_precision;

  };
}
#endif
