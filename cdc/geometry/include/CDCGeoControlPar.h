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

      static CDCGeoControlPar* m_pntr;  /*!< Pointer that saves the instance of this class. */
    };

  } // end of namespace CDC
} // end of namespace Belle2

#endif
