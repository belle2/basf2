/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

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
        For network specific parameters you can give either a list with
       * values for each network, or a single value that will be used for all.
       * The ranges are also valid if nPhi * nPt * nTheta * nPattern = nMLPs
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

      /** bit width etc. constant in each node */
      std::vector<std::vector<int>> total_bit_bias;
      std::vector<std::vector<int>> int_bit_bias;
      std::vector<std::vector<bool>> is_signed_bias;
      std::vector<std::vector<int>> rounding_bias;
      std::vector<std::vector<int>> saturation_bias;
      std::vector<std::vector<int>> total_bit_accum;
      std::vector<std::vector<int>> int_bit_accum;
      std::vector<std::vector<bool>> is_signed_accum;
      std::vector<std::vector<int>> rounding_accum;
      std::vector<std::vector<int>> saturation_accum;
      std::vector<std::vector<int>> total_bit_weight;
      std::vector<std::vector<int>> int_bit_weight;
      std::vector<std::vector<bool>> is_signed_weight;
      std::vector<std::vector<int>> rounding_weight;
      std::vector<std::vector<int>> saturation_weight;
      std::vector<std::vector<int>> total_bit_relu;
      std::vector<std::vector<int>> int_bit_relu;
      std::vector<std::vector<bool>> is_signed_relu;
      std::vector<std::vector<int>> rounding_relu;
      std::vector<std::vector<int>> saturation_relu;
      std::vector<std::vector<int>> total_bit;
      std::vector<std::vector<int>> int_bit;
      std::vector<std::vector<bool>> is_signed;
      std::vector<std::vector<int>> rounding;
      std::vector<std::vector<int>> saturation;
      std::vector<std::vector<std::vector<int>>> W_input;
      std::vector<std::vector<std::vector<int>>> I_input;
    };



    /** Default constructor. */
    GRLNeuro() {}

    /** Default destructor. */
    virtual ~GRLNeuro() {}

    /** Set parameters and get some network independent parameters. */
    void initialize(const Parameters& p);

    /** return reference to a neural network */
    GRLMLP& operator[](unsigned index) { return m_MLPs[index]; }
    /** return const reference to a neural network */
    const GRLMLP& operator[](unsigned index) const { return m_MLPs[index]; }
    /** return number of neural networks */
    unsigned nSectors() const { return m_MLPs.size(); }

    /** Save MLPs to file.
     * @param filename name of the TFile to write to
     * @param arrayname name of the TObjArray holding the MLPs in the file
     */
    void save(const std::string& filename, const std::string& arrayname = "MLPs");
    /** Load MLPs from file.
     * @param isector index of the MLP
     * @param wfilename name of the TFile to read from
     * @param bfilename name of the TObjArray holding the MLPs in the file
     * @return true if the MLPs were loaded correctly
     */
    bool load(unsigned isector, const std::string& wfilename, const std::string& bfilename);

    bool load(unsigned isector, std::vector<float> warray, std::vector<float> barray);//add
    /** Run an expert MLP.
     * @param isector index of the MLP
     * @param input vector of input values
     * @return output values (classifier) */
    float runMLP(unsigned isector,  const std::vector<float>& input);

  private:
    /** List of networks */
    std::vector<GRLMLP> m_MLPs = {};

  };
}
#endif
