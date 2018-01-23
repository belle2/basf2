/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                                             *
 * Author: The Belle II Collaboration                                          *
 * Contributors: CDC group                                                      *
 *                                                                                              *
 * This software is provided "as is" without any warranty.         *
 **************************************************************************/

#ifndef CDCGEOCONTROLPAR_H
#define CDCGEOCONTROLPAR_H

#include <string>

namespace Belle2 {
  namespace CDC {
    //! The Class for CDC Geometry Control Parameters
    /*! This class provides control paramters for CDC geometry.
    */
    class CDCGeoControlPar {

    public:

      //! Destructor
      virtual ~CDCGeoControlPar();

      //! Static method to get a reference to the CDCGeoControlPar instance.
      /*!
      \return A reference to an instance of this class.
      */
      static CDCGeoControlPar& getInstance();

      /**
       * Set debug flag
       */
      void setDebug(bool onoff)
      {
        m_debug = onoff;
      }

      /**
       * Set printMaterialTable flag
       */
      void setPrintMaterialTable(bool onoff)
      {
        m_printMaterialTable = onoff;
      }

      /**
       * Set material defition mode
       */
      void setMaterialDefinitionMode(int input)
      {
        m_materialDefinitionMode = input;
      }

      /**
       * Set sense wire zpos mode
       */
      void setSenseWireZposMode(int input)
      {
        m_senseWireZposMode = input;
      }

      /**
       * Set displacement switch
       */
      void setDisplacement(bool input)
      {
        m_displacement = input;
      }

      /**
       * Set alignment switch
       */
      void setAlignment(bool input)
      {
        m_alignment = input;
      }

      /**
       * Set misalignment switch
       */
      void setMisalignment(bool input)
      {
        m_misalignment = input;
      }

      /**
       * Set input type for wire displacement
       */
      void setDisplacementInputType(bool input)
      {
        m_displacementInputType = input;
      }

      /**
       * Set input type for wire alignment
       */
      void setAlignmentInputType(bool input)
      {
        m_alignmentInputType = input;
      }

      /**
       * Set input type for wire misalignment
       */
      void setMisalignmentInputType(bool input)
      {
        m_misalignmentInputType = input;
      }

      /**
       * Set input type for xt
       */
      void setXtInputType(bool input)
      {
        m_xtInputType = input;
      }

      /**
       * Set input type for sigma
       */
      void setSigmaInputType(bool input)
      {
        m_sigmaInputType = input;
      }

      /**
       * Set input type for prop. speed
       */
      void setPropSpeedInputType(bool input)
      {
        m_propSpeedInputType = input;
      }

      /**
       * Set input type for t0
       */
      void setT0InputType(bool input)
      {
        m_t0InputType = input;
      }

      /**
       * Set input type for time-walk
       */
      void setTwInputType(bool input)
      {
        m_twInputType = input;
      }

      /**
       * Set input type for bad wire
       */
      void setBwInputType(bool input)
      {
        m_bwInputType = input;
      }

      /**
       * Set input type for channel map
       */
      void setChMapInputType(bool input)
      {
        m_chMapInputType = input;
      }

      /**
       * Set input file name for wire displacement
       */
      void setDisplacementFile(std::string& input)
      {
        m_displacementFile = input;
      }

      /**
       * Set input file name for wire alignment
       */
      void setAlignmentFile(std::string& input)
      {
        m_alignmentFile = input;
      }

      /**
       * Set input file name for wire misalignment
       */
      void setMisalignmentFile(std::string& input)
      {
        m_misalignmentFile = input;
      }

      /**
       * Set input file name for xt-relation
       */
      void setXtFile(std::string& input)
      {
        m_xtFile = input;
      }

      /**
       * Set input file name for sigma
       */
      void setSigmaFile(std::string& input)
      {
        m_sigmaFile = input;
      }

      /**
       * Set input file name for prop-speed
       */
      void setPropSpeedFile(std::string& input)
      {
        m_propSpeedFile = input;
      }

      /**
       * Set input file name for t0
       */
      void setT0File(std::string& input)
      {
        m_t0File = input;
      }

      /**
       * Set input file name for time-walk
       */
      void setTwFile(std::string& input)
      {
        m_twFile = input;
      }

      /**
       * Set input file name for bad wire
       */
      void setBwFile(std::string& input)
      {
        m_bwFile = input;
      }

      /**
       * Set input file name for channel map
       */
      void setChMapFile(std::string& input)
      {
        m_chMapFile = input;
      }

      /**
       * Set max. space resolution (cm)
       */
      void setMaxSpaceResolution(double input)
      {
        m_maxSpaceResol = input;
      }

      /**
       * Set mapper geometry flag
       */
      void setMapperGeometry(bool torf)
      {
        m_mapperGeometry = torf;
      }

      /**
       * Set mapper phi-angle (deg.)
       */
      void setMapperPhiAngle(double phi)
      {
        m_mapperPhiAngle = phi;
      }


      /**
       * Get debug flag
       */
      bool getDebug() const
      {
        return m_debug;
      }

      /**
       * Get printMaterialTable flag
       */
      bool getPrintMaterialTable() const
      {
        return m_printMaterialTable;
      }

      /**
       * Get material definition mode
       */
      double getMaterialDefinitionMode() const
      {
        return m_materialDefinitionMode;
      }

      /**
       * Get sense wire z position mode
       */
      int getSenseWireZposMode() const
      {
        return m_senseWireZposMode;
      }

      /**
       * Get displacement switch
       */
      bool getDisplacement() const
      {
        return m_displacement;
      }

      /**
       * Get alignment switch
       */
      bool getAlignment() const
      {
        return m_alignment;
      }

      /**
       * Get misalignment switch
       */
      bool getMisalignment() const
      {
        return m_misalignment;
      }

      /**
       * Get input type for wire displacement
       */
      bool getDisplacementInputType()
      {
        return m_displacementInputType;
      }

      /**
       * Get input type for wire alignment
       */
      bool getAlignmentInputType()
      {
        return m_alignmentInputType;
      }

      /**
       * Get input type for wire misalignment
       */
      bool getMisalignmentInputType()
      {
        return m_misalignmentInputType;
      }

      /**
       * Get input type for xt
       */
      bool getXtInputType()
      {
        return m_xtInputType;
      }

      /**
       * Get input type for sigma
       */
      bool getSigmaInputType()
      {
        return m_sigmaInputType;
      }

      /**
       * Get input type for prop. speed
       */
      bool getPropSpeedInputType()
      {
        return m_propSpeedInputType;
      }

      /**
       * Get input type for t0
       */
      bool getT0InputType()
      {
        return m_t0InputType;
      }

      /**
       * Get input type for time-walk
       */
      bool getTwInputType()
      {
        return m_twInputType;
      }

      /**
       * Get input type for bad wire
       */
      bool getBwInputType()
      {
        return m_bwInputType;
      }

      /**
       * Get input type for channel map
       */
      bool getChMapInputType()
      {
        return m_chMapInputType;
      }

      /**
       * Get input file name for wire displacement
       */
      std::string getDisplacementFile() const
      {
        return m_displacementFile;
      }

      /**
       * Get input file name for wire alignment
       */
      std::string getAlignmentFile() const
      {
        return m_alignmentFile;
      }

      /**
       * Get input file name for wire misalignment
       */
      std::string getMisalignmentFile() const
      {
        return m_misalignmentFile;
      }

      /**
       * Get input file name for xt-relation
       */
      std::string getXtFile() const
      {
        return m_xtFile;
      }

      /**
       * Get input file name for sigma
       */
      std::string getSigmaFile() const
      {
        return m_sigmaFile;
      }

      /**
       * Get input file name for prop-speed
       */
      std::string getPropSpeedFile() const
      {
        return m_propSpeedFile;
      }

      /**
       * Get input file name for t0
       */
      std::string getT0File() const
      {
        return m_t0File;
      }

      /**
       * Get input file name for time-walk
       */
      std::string getTwFile() const
      {
        return m_twFile;
      }

      /**
       * Get input file name for badwire
       */
      std::string getBwFile() const
      {
        return m_bwFile;
      }

      /**
       * Get input file name for channel map
       */
      std::string getChMapFile() const
      {
        return m_chMapFile;
      }

      /**
       * Get max. space resolution
       */
      double getMaxSpaceResolution()
      {
        return m_maxSpaceResol;
      }

      /**
       * Get mapper geometry flag
       */
      bool getMapperGeometry()
      {
        return m_mapperGeometry;
      }

      /**
       * Get mapper phi-angle
       */
      double getMapperPhiAngle()
      {
        return m_mapperPhiAngle;
      }

    private:
      /** Singleton class */
      CDCGeoControlPar();
      /** Singleton class */
      CDCGeoControlPar(const CDCGeoControlPar&);
      /** Singleton class */
      CDCGeoControlPar& operator=(const CDCGeoControlPar&);

      bool m_debug = false;                    /*!< Switch for debug printing. */
      bool m_printMaterialTable = false;       /*!< Switch for printing the G4 material table. */

      int m_materialDefinitionMode = 0;     /*!< Material definition mode. */
      int m_senseWireZposMode = 1;     /*!< Sense wire z postion mode. */

      bool m_displacement = true;  /**< Switch for displacement. */
      bool m_alignment = true;  /**< Switch for alignment. */
      bool m_misalignment = false;  /**< Switch for misalignment. */

      bool m_displacementInputType = true;  /**< Input type for displacement. */
      bool m_alignmentInputType = true;  /**< Input type for alignment. */
      bool m_misalignmentInputType = true;  /**< Input type for misalignment. */
      bool m_xtInputType = true;  /**< Input type for xt. */
      bool m_sigmaInputType = true;  /**< Input type for sigma. */
      bool m_propSpeedInputType = true;  /**< Input type for prop. speed. */
      bool m_t0InputType = true;  /**< Input type for t0. */
      bool m_twInputType = true;  /**< Input type for time-walk. */
      bool m_bwInputType = true;  /**< Input type for bad wire. */
      bool m_chMapInputType = true;  /**< Input type for channel map. */

      double m_maxSpaceResol = 2.5 * 0.0130; /**< Max. space resolution allowed (cm) */
      bool m_mapperGeometry = false;  /**< B-field mapper geometry flag. */
      double m_mapperPhiAngle = 16.7; /**< B-field mapper phi-angle (deg). */

      std::string m_displacementFile = "displacement_v2.2.1.dat";  /**< Displacement file. */
      std::string m_alignmentFile = "alignment_v2.dat";  /**< Alignment file. */
      std::string m_misalignmentFile = "misalignment_v2.dat";  /**< Misalignment file. */
      std::string m_xtFile = "xt_v3_chebyshev.dat.gz";  /**< Xt file. */
      std::string m_sigmaFile = "sigma_v2.dat";  /**< Sigma file. */
      std::string m_propSpeedFile = "propspeed_v0.dat";  /**< Prop-apeed file. */
      std::string m_t0File = "t0_v1.dat";  /**< T0 file. */
      std::string m_twFile = "tw_off.dat";  /**< Time walk file. */
      std::string m_bwFile = "badwire_v1.dat";  /**< Bad wire file. */
      std::string m_chMapFile = "ch_map.dat";  /**< Channel map file. */

      static CDCGeoControlPar* m_pntr;  /*!< Pointer that saves the instance of this class. */
    };

  } // end of namespace CDC
} // end of namespace Belle2

#endif
