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
       * Set input file name for wire displacement
       */
      void setDisplacementFile(std::string input)
      {
        m_displacementFile = input;
      }

      /**
       * Set input file name for wire alignment
       */
      void setAlignmentFile(std::string input)
      {
        m_alignmentFile = input;
      }

      /**
       * Set input file name for wire misalignment
       */
      void setMisalignmentFile(std::string input)
      {
        m_misalignmentFile = input;
      }

      /**
       * Set input file name for xt-relation
       */
      void setXtFile(std::string input)
      {
        m_xtFile = input;
      }

      /**
       * Set input file name for sigma
       */
      void setSigmaFile(std::string input)
      {
        m_sigmaFile = input;
      }

      /**
       * Set input file name for prop-speed
       */
      void setPropSpeedFile(std::string input)
      {
        m_propSpeedFile = input;
      }

      /**
       * Set input file name for t0
       */
      void setT0File(std::string input)
      {
        m_t0File = input;
      }

      /**
       * Set input file name for time-walk
       */
      void setTwFile(std::string input)
      {
        m_twFile = input;
      }

      /**
       * Set input file name for bad wire
       */
      void setBwFile(std::string input)
      {
        m_bwFile = input;
      }

      /**
       * Set input file name for channel map
       */
      void setChMapFile(std::string input)
      {
        m_chMapFile = input;
      }


      /**
       * Get debug flag
       */
      bool getDebug() const
      {
        return m_debug;
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

    private:
      /** Singleton class */
      CDCGeoControlPar();
      /** Singleton class */
      CDCGeoControlPar(const CDCGeoControlPar&);
      /** Singleton class */
      CDCGeoControlPar& operator=(const CDCGeoControlPar&);

      bool m_debug = false;                    /*!< Switch for debug printing. */

      int m_materialDefinitionMode = 0;     /*!< Material definition mode. */
      int m_senseWireZposMode = 1;     /*!< Sense wire z postion mode. */

      std::string m_displacementFile = "displacement_v1.1.dat";  /**< Displacement file. */
      std::string m_alignmentFile = "alignment_v2.dat";  /**< Alignment file. */
      std::string m_misalignmentFile = "misalignment_v2.dat";  /**< Misalignment file. */
      std::string m_xtFile = "xt_v3.dat";  /**< Xt file. */
      std::string m_sigmaFile = "sigma_v1.dat";  /**< Sigma file. */
      std::string m_propSpeedFile = "propspeed_v0.dat";  /**< Prop-apeed file. */
      std::string m_t0File = "t0.dat";  /**< T0 file. */
      std::string m_twFile = "tw_off.dat";  /**< Time walk file. */
      std::string m_bwFile = "badwire_v1.dat";  /**< Bad wire file. */
      std::string m_chMapFile = "ch_map.dat";  /**< Channel map file. */

      static CDCGeoControlPar* m_pntr;  /*!< Pointer that saves the instance of this class. */
    };

  } // end of namespace CDC
} // end of namespace Belle2

#endif
