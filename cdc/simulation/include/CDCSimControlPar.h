/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                                             *
 * Author: The Belle II Collaboration                                          *
 * Contributors: CDC group                                                      *
 *                                                                                              *
 * This software is provided "as is" without any warranty.         *
 **************************************************************************/

#ifndef CDCSIMCONTROLPAR_H
#define CDCSIMCONTROLPAR_H

namespace Belle2 {
  namespace CDC {
    //! The Class for CDC Simulation Control Parameters
    /*! This class provides control paramters for CDC simulation.
    */
    class CDCSimControlPar {

    public:

      //! Destructor
      virtual ~CDCSimControlPar();

      //! Static method to get a reference to the CDCSimControlPar instance.
      /*!
      \return A reference to an instance of this class.
      */
      static CDCSimControlPar& getInstance();

      /**
       * Set wiresag flag
       */
      void setWireSag(bool onoff)
      {
        m_wireSag = onoff;
      }

      /**
       * Set modified left/right flag
       */
      void setModLeftRightFlag(bool onoff)
      {
        m_modLeftRightFlag = onoff;
      }

      /**
       * Set debug flag
       */
      void setDebug(bool onoff)
      {
        m_debug = onoff;
      }

      /**
       * Set threshold for Energy Deposit;
       */
      void setThresholdEnergyDeposit(double input)
      {
        m_thresholdEnergyDeposit = input;
      }

      /**
       * Set minimum track length
       */
      void setMinTrackLength(double input)
      {
        m_minTrackLength = input;
      }

      /**
       * Get wiresag flag
       */
      bool getWireSag()
      {
        return m_wireSag;
      }

      /**
       * Get modified left/right flag
       */
      bool getModLeftRightFlag()
      {
        return m_modLeftRightFlag;
      }

      /**
       * Get debug flag
       */
      bool getDebug()
      {
        return m_debug;
      }

      /**
       * Get threshold for Energy Deposit;
       */
      double getThresholdEnergyDeposit()
      {
        return m_thresholdEnergyDeposit;
      }

      /**
       * Get minimum track length
       */
      double getMinTrackLength()
      {
        return m_minTrackLength;
      }

    private:
      /** Singleton class */
      CDCSimControlPar();
      /** Singleton class */
      CDCSimControlPar(const CDCSimControlPar&);
      /** Singleton class */
      CDCSimControlPar& operator=(const CDCSimControlPar&);

      bool m_wireSag = true;                       /*!< Switch for sense wire sag */
      bool m_modLeftRightFlag = false;   /*!< Switch for modified left/right flag */
      bool m_debug = false;                    /*!< Switch for debug printing. */

      double m_thresholdEnergyDeposit = 0.;  /*!< Energy thresh. for G4 step (GeV) */
      double m_minTrackLength = 15.;            /*!< Minimum track length for G4 step (cm) */

      static CDCSimControlPar* m_pntr;  /*!< Pointer that saves the instance of this class. */
    };

  } // end of namespace CDC
} // end of namespace Belle2

#endif
