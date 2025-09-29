/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TObject.h>

namespace Belle2 {



  /** The payload class for GRL parameters
   *
   */


  class TRGGRLConfig: public TObject {
  public:

    /** Default constructor */
    TRGGRLConfig(): m_ecltaunn_threshold{-1} {}

    /** Getter functions*/
    float get_ecltaunn_threshold() const
    {
      return m_ecltaunn_threshold;
    }
    unsigned get_ecltaunn_nMLP() const
    {
      return m_ecltaunn_nMLP;
    }
    bool get_ecltaunn_multiplyHidden() const
    {
      return m_ecltaunn_multiplyHidden;
    }
    std::vector<std::vector<float>> get_ecltaunn_nHidden() const
    {
      return m_ecltaunn_nHidden;
    }
    unsigned get_ecltaunn_n_cdc_sector() const
    {
      return m_ecltaunn_n_cdc_sector;
    }
    unsigned get_ecltaunn_n_ecl_sector() const
    {
      return m_ecltaunn_n_ecl_sector;
    }
    std::vector<float> get_ecltaunn_i_cdc_sector() const
    {
      return m_ecltaunn_i_cdc_sector;
    }
    std::vector<float> get_ecltaunn_i_ecl_sector() const
    {
      return m_ecltaunn_i_ecl_sector;
    }
    std::vector<std::vector<float>> get_ecltaunn_weight() const
    {
      return m_ecltaunn_weight;
    }
    std::vector<std::vector<float>> get_ecltaunn_bias() const
    {
      return m_ecltaunn_bias;
    }

    /** Setter functions*/
    void set_ecltaunn_threshold(float i)
    {
      m_ecltaunn_threshold = i;
    }
    void set_ecltaunn_nMLP(unsigned i)
    {
      m_ecltaunn_nMLP = i;
    };
    void set_ecltaunn_multiplyHidden(bool i)
    {
      m_ecltaunn_multiplyHidden = i;
    };
    void set_ecltaunn_nHidden(std::vector<std::vector<float>> i)
    {
      m_ecltaunn_nHidden = i;
    };
    void set_ecltaunn_n_cdc_sector(unsigned i)
    {
      m_ecltaunn_n_cdc_sector = i;
    };
    void set_ecltaunn_n_ecl_sector(unsigned i)
    {
      m_ecltaunn_n_ecl_sector = i;
    };
    void set_ecltaunn_i_cdc_sector(std::vector<float> i)
    {
      m_ecltaunn_i_cdc_sector = i;
    };
    void set_ecltaunn_i_ecl_sector(std::vector<float> i)
    {
      m_ecltaunn_i_ecl_sector = i;
    };
    void set_ecltaunn_weight(std::vector<std::vector<float>> i)
    {
      m_ecltaunn_weight = i;
    };
    void set_ecltaunn_bias(std::vector<std::vector<float>> i)
    {
      m_ecltaunn_bias = i;
    };

  private:

    /** MVA Threshold of ecltaunn bit */
    float m_ecltaunn_threshold;

    /** Number of networks.
      For network specific parameters you can give either a list with
     * values for each network, or a single value that will be used for all.
     * The ranges are also valid if nPhi * nPt * nTheta * nPattern = nMLPs
     */
    unsigned m_ecltaunn_nMLP;

    /** If true, multiply nHidden with number of input nodes. */
    bool m_ecltaunn_multiplyHidden;

    /** Number of nodes in each hidden layer for all networks
     * or factor to multiply with number of inputs.
     * The number of layers is derived from the shape.
     */
    std::vector<std::vector<float>> m_ecltaunn_nHidden;

    /** Number of CDC sectors. */
    unsigned m_ecltaunn_n_cdc_sector;
    std::vector<float> m_ecltaunn_i_cdc_sector;

    /** Number of ECL sectors. */
    unsigned m_ecltaunn_n_ecl_sector;
    std::vector<float> m_ecltaunn_i_ecl_sector;

    /** weight and bias data in each nMLP*/
    std::vector<std::vector<float>> m_ecltaunn_weight;
    std::vector<std::vector<float>> m_ecltaunn_bias;

    ClassDef(TRGGRLConfig, 4);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2
