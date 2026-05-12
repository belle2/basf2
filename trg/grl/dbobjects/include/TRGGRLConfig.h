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
    TRGGRLConfig() {}

    /** Getter functions*/
    std::vector<std::vector<float>> get_ecltaunn_threshold() const
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
    std::vector<unsigned> get_ecltaunn_nOutput() const
    {
      return m_ecltaunn_nOutput;
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
    std::vector<std::vector<int>> get_ecltaunn_total_bit_bias() const
    {
      return m_ecltaunn_total_bit_bias;
    }
    std::vector<std::vector<int>> get_ecltaunn_int_bit_bias() const
    {
      return m_ecltaunn_int_bit_bias;
    }
    std::vector<std::vector<bool>> get_ecltaunn_is_signed_bias() const
    {
      return m_ecltaunn_is_signed_bias;
    }
    std::vector<std::vector<int>> get_ecltaunn_rounding_bias() const
    {
      return m_ecltaunn_rounding_bias;
    }
    std::vector<std::vector<int>> get_ecltaunn_saturation_bias() const
    {
      return m_ecltaunn_saturation_bias;
    }
    std::vector<std::vector<int>> get_ecltaunn_total_bit_accum() const
    {
      return m_ecltaunn_total_bit_accum;
    }
    std::vector<std::vector<int>> get_ecltaunn_int_bit_accum() const
    {
      return m_ecltaunn_int_bit_accum;
    }
    std::vector<std::vector<bool>> get_ecltaunn_is_signed_accum() const
    {
      return m_ecltaunn_is_signed_accum;
    }
    std::vector<std::vector<int>> get_ecltaunn_rounding_accum() const
    {
      return m_ecltaunn_rounding_accum;
    }
    std::vector<std::vector<int>> get_ecltaunn_saturation_accum() const
    {
      return m_ecltaunn_saturation_accum;
    }
    std::vector<std::vector<int>> get_ecltaunn_total_bit_weight() const
    {
      return m_ecltaunn_total_bit_weight;
    }
    std::vector<std::vector<int>> get_ecltaunn_int_bit_weight() const
    {
      return m_ecltaunn_int_bit_weight;
    }
    std::vector<std::vector<bool>> get_ecltaunn_is_signed_weight() const
    {
      return m_ecltaunn_is_signed_weight;
    }
    std::vector<std::vector<int>> get_ecltaunn_rounding_weight() const
    {
      return m_ecltaunn_rounding_weight;
    }
    std::vector<std::vector<int>> get_ecltaunn_saturation_weight() const
    {
      return m_ecltaunn_saturation_weight;
    }
    std::vector<std::vector<int>> get_ecltaunn_total_bit_relu() const
    {
      return m_ecltaunn_total_bit_relu;
    }
    std::vector<std::vector<int>> get_ecltaunn_int_bit_relu() const
    {
      return m_ecltaunn_int_bit_relu;
    }
    std::vector<std::vector<bool>> get_ecltaunn_is_signed_relu() const
    {
      return m_ecltaunn_is_signed_relu;
    }
    std::vector<std::vector<int>> get_ecltaunn_rounding_relu() const
    {
      return m_ecltaunn_rounding_relu;
    }
    std::vector<std::vector<int>> get_ecltaunn_saturation_relu() const
    {
      return m_ecltaunn_saturation_relu;
    }
    std::vector<std::vector<int>> get_ecltaunn_total_bit() const
    {
      return m_ecltaunn_total_bit;
    }
    std::vector<std::vector<int>> get_ecltaunn_int_bit() const
    {
      return m_ecltaunn_int_bit;
    }
    std::vector<std::vector<bool>> get_ecltaunn_is_signed() const
    {
      return m_ecltaunn_is_signed;
    }
    std::vector<std::vector<int>> get_ecltaunn_rounding() const
    {
      return m_ecltaunn_rounding;
    }
    std::vector<std::vector<int>> get_ecltaunn_saturation() const
    {
      return m_ecltaunn_saturation;
    }
    std::vector<std::vector<std::vector<int>>> get_ecltaunn_W_input() const
    {
      return m_ecltaunn_W_input;
    }
    std::vector<std::vector<std::vector<int>>> get_ecltaunn_I_input() const
    {
      return m_ecltaunn_I_input;
    }


    /** Setter functions*/
    void set_ecltaunn_threshold(std::vector<std::vector<float>> i)
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
    void set_ecltaunn_nOutput(std::vector<unsigned> i)
    {
      m_ecltaunn_nOutput = i;
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
    void set_ecltaunn_total_bit_bias(std::vector<std::vector<int>> i)
    {
      m_ecltaunn_total_bit_bias = i;
    };
    void set_ecltaunn_int_bit_bias(std::vector<std::vector<int>> i)
    {
      m_ecltaunn_int_bit_bias = i;
    };
    void set_ecltaunn_is_signed_bias(std::vector<std::vector<bool>> i)
    {
      m_ecltaunn_is_signed_bias = i;
    };
    void set_ecltaunn_rounding_bias(std::vector<std::vector<int>> i)
    {
      m_ecltaunn_rounding_bias = i;
    };
    void set_ecltaunn_saturation_bias(std::vector<std::vector<int>> i)
    {
      m_ecltaunn_saturation_bias = i;
    };
    void set_ecltaunn_total_bit_accum(std::vector<std::vector<int>> i)
    {
      m_ecltaunn_total_bit_accum = i;
    };
    void set_ecltaunn_int_bit_accum(std::vector<std::vector<int>> i)
    {
      m_ecltaunn_int_bit_accum = i;
    };
    void set_ecltaunn_is_signed_accum(std::vector<std::vector<bool>> i)
    {
      m_ecltaunn_is_signed_accum = i;
    };
    void set_ecltaunn_rounding_accum(std::vector<std::vector<int>> i)
    {
      m_ecltaunn_rounding_accum = i;
    };
    void set_ecltaunn_saturation_accum(std::vector<std::vector<int>> i)
    {
      m_ecltaunn_saturation_accum = i;
    };
    void set_ecltaunn_total_bit_weight(std::vector<std::vector<int>> i)
    {
      m_ecltaunn_total_bit_weight = i;
    };
    void set_ecltaunn_int_bit_weight(std::vector<std::vector<int>> i)
    {
      m_ecltaunn_int_bit_weight = i;
    };
    void set_ecltaunn_is_signed_weight(std::vector<std::vector<bool>> i)
    {
      m_ecltaunn_is_signed_weight = i;
    };
    void set_ecltaunn_rounding_weight(std::vector<std::vector<int>> i)
    {
      m_ecltaunn_rounding_weight = i;
    };
    void set_ecltaunn_saturation_weight(std::vector<std::vector<int>> i)
    {
      m_ecltaunn_saturation_weight = i;
    };
    void set_ecltaunn_total_bit_relu(std::vector<std::vector<int>> i)
    {
      m_ecltaunn_total_bit_relu = i;
    };
    void set_ecltaunn_int_bit_relu(std::vector<std::vector<int>> i)
    {
      m_ecltaunn_int_bit_relu = i;
    };
    void set_ecltaunn_is_signed_relu(std::vector<std::vector<bool>> i)
    {
      m_ecltaunn_is_signed_relu = i;
    };
    void set_ecltaunn_rounding_relu(std::vector<std::vector<int>> i)
    {
      m_ecltaunn_rounding_relu = i;
    };
    void set_ecltaunn_saturation_relu(std::vector<std::vector<int>> i)
    {
      m_ecltaunn_saturation_relu = i;
    };
    void set_ecltaunn_total_bit(std::vector<std::vector<int>> i)
    {
      m_ecltaunn_total_bit = i;
    };
    void set_ecltaunn_int_bit(std::vector<std::vector<int>> i)
    {
      m_ecltaunn_int_bit = i;
    };
    void set_ecltaunn_is_signed(std::vector<std::vector<bool>> i)
    {
      m_ecltaunn_is_signed = i;
    };
    void set_ecltaunn_rounding(std::vector<std::vector<int>> i)
    {
      m_ecltaunn_rounding = i;
    };
    void set_ecltaunn_saturation(std::vector<std::vector<int>> i)
    {
      m_ecltaunn_saturation = i;
    };
    void set_ecltaunn_W_input(std::vector<std::vector<std::vector<int>>> i)
    {
      m_ecltaunn_W_input = i;
    };
    void set_ecltaunn_I_input(std::vector<std::vector<std::vector<int>>> i)
    {
      m_ecltaunn_I_input = i;
    };



  private:

    /** MVA Threshold of ecltaunn bit */
    std::vector<std::vector<float>> m_ecltaunn_threshold;

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

    /** Number of output node */
    std::vector<unsigned> m_ecltaunn_nOutput;

    /** Number of CDC sectors. */
    unsigned m_ecltaunn_n_cdc_sector;
    std::vector<float> m_ecltaunn_i_cdc_sector;

    /** Number of ECL sectors. */
    unsigned m_ecltaunn_n_ecl_sector;
    std::vector<float> m_ecltaunn_i_ecl_sector;

    /** weight and bias data in each nMLP*/
    std::vector<std::vector<float>> m_ecltaunn_weight;
    std::vector<std::vector<float>> m_ecltaunn_bias;
    /** bit width etc. constant in each node */
    std::vector<std::vector<int>> m_ecltaunn_total_bit_bias;
    std::vector<std::vector<int>> m_ecltaunn_int_bit_bias;
    std::vector<std::vector<bool>> m_ecltaunn_is_signed_bias;
    std::vector<std::vector<int>> m_ecltaunn_rounding_bias;
    std::vector<std::vector<int>> m_ecltaunn_saturation_bias;
    std::vector<std::vector<int>> m_ecltaunn_total_bit_accum;
    std::vector<std::vector<int>> m_ecltaunn_int_bit_accum;
    std::vector<std::vector<bool>> m_ecltaunn_is_signed_accum;
    std::vector<std::vector<int>> m_ecltaunn_rounding_accum;
    std::vector<std::vector<int>> m_ecltaunn_saturation_accum;
    std::vector<std::vector<int>> m_ecltaunn_total_bit_weight;
    std::vector<std::vector<int>> m_ecltaunn_int_bit_weight;
    std::vector<std::vector<bool>> m_ecltaunn_is_signed_weight;
    std::vector<std::vector<int>> m_ecltaunn_rounding_weight;
    std::vector<std::vector<int>> m_ecltaunn_saturation_weight;
    std::vector<std::vector<int>> m_ecltaunn_total_bit_relu;
    std::vector<std::vector<int>> m_ecltaunn_int_bit_relu;
    std::vector<std::vector<bool>> m_ecltaunn_is_signed_relu;
    std::vector<std::vector<int>> m_ecltaunn_rounding_relu;
    std::vector<std::vector<int>> m_ecltaunn_saturation_relu;
    std::vector<std::vector<int>> m_ecltaunn_total_bit;
    std::vector<std::vector<int>> m_ecltaunn_int_bit;
    std::vector<std::vector<bool>> m_ecltaunn_is_signed;
    std::vector<std::vector<int>> m_ecltaunn_rounding;
    std::vector<std::vector<int>> m_ecltaunn_saturation;
    std::vector<std::vector<std::vector<int>>> m_ecltaunn_W_input;
    std::vector<std::vector<std::vector<int>>> m_ecltaunn_I_input;


    ClassDef(TRGGRLConfig, 8);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2
