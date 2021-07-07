/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

namespace Belle2 {
  /**
   * Create Field maps of the Belle II magnetic field used in the simulation
   */
  class CreateFieldMapModule : public Module {

  public:
    /** Types of Fieldmap to be created */
    enum EFieldTypes {
      c_XY, /**< scan along XY plane */
      c_ZX, /**< scan along ZX plane */
      c_ZY, /**< scan along ZY plane */
      c_ZR /**< scan along Z and R, averaging over phi */
    };

    /**
     * Constructor: Sets the description, the properties and the parameters of
     * the module.
     */
    CreateFieldMapModule();

    /** Check input parameters */
    virtual void initialize() override;

    /** Create the fieldmap */
    virtual void beginRun() override;

  private:
    /** output filename for the fieldmap */
    std::string m_filename;
    /** type of the fieldmap (zx, zy, zr) */
    std::string m_type;
    /** number of steps along the first coordinate */
    int m_nU {0};
    /** start value for the first coordinate */
    double m_minU {0};
    /** end value for the first coordinate */
    double m_maxU {0};
    /** number of steps along the second coordinate */
    int m_nV {0};
    /** start value for the first coordinate */
    double m_minV {0};
    /** end value for the first coordinate */
    double m_maxV {0};
    /** phi rotation when sampling magnetic field (value of pi will convert ZX
     * to ZY scan) */
    double m_phi {0};
    /** offset on the third coordinate */
    double m_wOffset {0};
    /** number of steps in phi */
    int m_nPhi{180};
    /** if true create a TTree with all sampled points */
    bool m_createTree{false};
  };
}
