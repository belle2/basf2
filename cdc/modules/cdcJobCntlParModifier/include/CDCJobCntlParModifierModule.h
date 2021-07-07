/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//basf2 framework headers
#include <framework/core/Module.h>

//cdc package headers
#include <cdc/simulation/CDCSimControlPar.h>
#include <cdc/geometry/CDCGeoControlPar.h>

//C++/C standard lib elements.
#include <string>

namespace Belle2 {

  /** The Class for Detailed Digitization of CDC.
   */
  class CDCJobCntlParModifierModule : public Module {

  public:
    /** Constructor.*/
    CDCJobCntlParModifierModule();

    /** Initialize */
    void initialize() override;

    /** Actual digitization of all hits in the CDC. */
    void event() override;

  private:

    CDC::CDCSimControlPar& m_scp;  /*!< Reference to CDCSim... */
    CDC::CDCGeoControlPar& m_gcp;  /*!< Reference to CDCGeo... */

    //For Simulation
    bool m_timeWalk;                        /*!< Switch for time walk in translator */
    bool m_wireSag;                         /*!< Switch for sense wire sag */
    bool m_modLeftRightFlag;                /*!< Switch for modified left/right flag */
    bool m_debug4Sim;                          /*!< Switch for debug printing. */
    double m_thresholdEnergyDeposit;  /*!< Energy thresh. for G4 step (GeV) */
    double m_minTrackLength;              /*!< Minimum track length for G4 step (cm) */
    double m_maxSpaceResol;   /**< Max. space resolution allowed (cm). */
    double m_addFudgeFactorForSigmaForData;   /**< Additional fudge factor for space resol. for data. */
    double m_addFudgeFactorForSigmaForMC;   /**< Additional fudge factor for space resol. for MC. */
    bool   m_mapperGeometry;  /**< Mapper geometry flag. */
    double m_mapperPhiAngle;  /**< Mapper phi-angle(deg). */

    //For Geometry
    bool m_debug4Geo;              /*!< Switch for debug printing. */
    bool m_printMaterialTable;     /*!< Switch for printing the G4 material table. */
    int m_materialDefinitionMode;  /*!< Material definition mode */
    int m_senseWireZposMode;    /*!< Sense wire Z pos. mode */

    bool m_displacement;  /**< Switch for displacement. */
    bool m_alignment;  /**< Switch for alignment. */
    bool m_misalignment;  /**< Switch for misalignment. */

    bool m_displacementInputType;  /**< Displacement input type. */
    bool m_alignmentInputType;  /**< Alignment input type. */
    bool m_misalignmentInputType;  /**< Misalignment input type. */
    bool m_xtInputType;   /**< Xt input type. */
    bool m_sigmaInputType;   /**< Sigma input type. */
    bool m_propSpeedInputType;  /**< Prop-apeed input type. */
    bool m_t0InputType;   /**< T0 input type. */
    bool m_twInputType;   /**< Time walk input type. */
    bool m_bwInputType;   /**< Bad wire input type. */
    bool m_chMapInputType;   /**< Channel map input type. */
    bool m_eDepToADCInputType;   /**< EDep-to-adc input type. */

    std::string m_displacementFile;  /**< Displacement file. */
    std::string m_alignmentFile;  /**< Alignment file. */
    std::string m_misalignmentFile;  /**< Misalignment file. */
    std::string m_xtFile;   /**< Xt file. */
    std::string m_sigmaFile;   /**< Sigma file. */
    std::string m_propSpeedFile;  /**< Prop-apeed file. */
    std::string m_t0File;   /**< T0 file. */
    std::string m_twFile;   /**< Time walk file. */
    std::string m_bwFile;   /**< Bad wire file. */
    std::string m_chMapFile;   /**< Channel map file. */
    std::string m_eDepToADCFile;   /**< EDep-to-adc file. */
  };

} // end of Belle2 namespace
