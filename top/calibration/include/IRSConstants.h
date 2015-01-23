/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef IRSCONSTANTS_H
#define IRSCONSTANTS_H

#include <top/calibration/ASICChannelConstants.h>
#include <vector>
#include <string>

namespace Belle2 {
  namespace TOP {

    /**
     * Calibration constants of TOP front-end electornics (type IRS) for full detector
     * Provides also interface to DB (read and write)
     * !!!! Under development !!!!
     */
    class IRSConstants {
    public:

      /**
       * array sizes
       */
      enum {c_NumBars = 16,      /**< number of TOP modules */
            c_NumChannels = 512  /**< number of channels per TOP module */
           };

      /**
       * Default constructor
       */
      IRSConstants(): m_valid(false), m_run(0), m_runFrom(0), m_runTo(0) {
        for (unsigned k = 0; k < c_NumBars; k++) {
          for (unsigned i = 0; i < c_NumChannels; i++) m_bar[k].push_back(0);
        }
      }

      /**
       * Destructor
       */
      ~IRSConstants() {
        for (unsigned k = 0; k < c_NumBars; k++) {
          auto& bar = m_bar[k];
          for (auto & channel : bar) {
            if (channel) delete channel;
          }
        }
      }

      /**
       * Set file name that is used for calibration constants
       * @param fileName root file name
       */
      void setFileName(const std::string& fileName) {m_fileName = fileName;}

      /**
       * Update calibration constants from DB
       * @param run run number
       * @param replace if true, replace the existing one with the one from DB
       * @return true on success
       */
      bool updateConstants(int run, bool replace = true);

      /**
       * Set constants of a single ASIC channel
       * Note: function takes ownership of a pointer
       * @param constants constants of a single ASIC channel
       * @param replace if true, replace the existing one with the one given as argument
       */
      void setConstants(ASICChannelConstants* constants, bool replace = false);

      /**
       * Write calibration constants for a given run number to DB
       * @param run run number
       * @param runFrom valid for run numbers from (incl)
       * @param runTo valid for run numbers to (excl)
       * @return true on success
       */
      bool writeConstants(int run, int runFrom = 0, int runTo = 999999);

      /**
       * Return file name that is used for calibration constants
       * @return file name
       */
      std::string getFileName() const {return m_fileName;}

      /**
       * Check if constants are valid
       * @return true if constants valid
       */
      bool isValid() const {return m_valid;}

      /**
       * Return run number with which the constants were updated
       * @return run number
       */
      int getRunNumber() const {return m_run;}

      /**
       * Return run number from which the constants are valid (including it)
       * @return run number
       */
      int getRunFrom() const {return m_runFrom;}

      /**
       * Return run number to which the constants are valid (excluding it)
       * @return run number
       */
      int getRunTo() const {return m_runTo;}

      /**
       * Return constants of ASIC channel
       * @param barID TOP module ID
       * @param channel hardware channel number
       * @return pointer to the object or NULL
       */
      const ASICChannelConstants* getConstants(int barID, unsigned channel) const {
        if (barID < 1 or barID > c_NumBars) return 0;
        barID--;
        auto& bar = m_bar[barID];
        if (channel >= bar.size()) return 0;
        return bar[channel];
      }

    private:

      std::string m_fileName; /**< root file name with calibration constants */
      bool m_valid;  /**< status of constants */
      int m_run;     /**< current run number for these constants */
      int m_runFrom; /**< constants valid from this run number on */
      int m_runTo;   /**< constants valid up to but excluding this run number */

      std::vector<ASICChannelConstants*> m_bar[c_NumBars]; /**< calibration constants */

    };

  } // end namespace TOP
} // end namespace Belle2

#endif
