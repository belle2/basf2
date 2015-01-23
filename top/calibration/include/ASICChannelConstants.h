/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ASICCHANNELCONSTANTS_H
#define ASICCHANNELCONSTANTS_H

#include <TObject.h>
#include <top/calibration/ASICWindowConstants.h>
#include <vector>

namespace Belle2 {
  namespace TOP {

    /**
     * Calibration constants of a singe ASIC channel: pedestals, gains and time axis
     */
    class ASICChannelConstants : public TObject {
    public:

      /**
       * Default constructor
       */
      ASICChannelConstants(): m_barID(0), m_channel(0), m_timeInterval(0) {
        for (unsigned i = 0; i < ASICWindowConstants::c_WindowSize; i++)
          m_timeAxis[i] = 0;
      }

      /**
       * Constructor with barID, hardware channel number and number of ASIC windows
       * @param barID bar ID
       * @param channel hardware channel number
       * @param numWindows number of active windows per ASIC
       */
      ASICChannelConstants(int barID, unsigned channel, int numWindows):
        m_barID(barID), m_channel(channel) {
        for (int i = 0; i < numWindows; i++) m_windows.push_back(NULL);

        float timeBin = 1 / 2.77; //TODO get correct value
        setTimeAxis(timeBin);
        setTimeInterval(timeBin * ASICWindowConstants::c_WindowSize);
      }

      /**
       * Destructor
       */
      ~ASICChannelConstants() {
        for (auto & window : m_windows) {
          if (window) delete window;
        }
      }

      /**
       * Set calibration constants of a single ASIC window.
       * @param windowConstants calibration constants of ASIC window
       * @return true on success
       */
      bool setAsicWindowConstants(const ASICWindowConstants& windowConstants) {
        unsigned i = windowConstants.getAsicWindow();
        if (i < m_windows.size()) {
          if (m_windows[i]) {
            *m_windows[i] = windowConstants;
          } else {
            m_windows[i] = new ASICWindowConstants(windowConstants);
          }
          return true;
        }
        return false;
      }

      /**
       * Set time interval between two consecutive ASIC windows
       * @param timeInterval time interval
       */
      void setTimeInterval(float timeInterval) {m_timeInterval = timeInterval;}

      /**
       * Set equidistant time axis
       * @param timeBin time bin
       */
      void setTimeAxis(float timeBin) {
        float t = 0;
        for (unsigned i = 0; i < ASICWindowConstants::c_WindowSize; i++) {
          m_timeAxis[i] = t;
          t += timeBin;
        }
      }

      /**
       * Return bar ID
       * @return bar ID
       */
      int getBarID() const {return m_barID;}


      /**
       * Return hardware channel number
       * @return channel number
       */
      unsigned getChannel() const {return m_channel;}

      /**
       * Return time interval between two consecutive ASIC windows
       * @return time interval
       */
      float getTimeInterval() const {return m_timeInterval;}

      /**
       * Return time axis with an offset added
       * @param offset offset to be added
       * @return time axis
       */
      std::vector<float> getTimeAxis(float offset = 0) const {
        std::vector<float> timeAxis;
        for (unsigned i = 0; i < ASICWindowConstants::c_WindowSize; i++) {
          timeAxis.push_back(m_timeAxis[i] + offset);
        }
        return timeAxis;
      }

      /**
       * Return number of ASIC windows
       * @return number of windows
       */
      unsigned getNumofWindows() const {
        return m_windows.size();
      }

      /**
       * Return calibration constants of i-th ASIC window
       * @param i ASIC window number
       * @return pointer to calibration constants or NULL
       */
      const ASICWindowConstants* getAsicWindowConstants(unsigned i) const {
        if (i < m_windows.size()) return m_windows[i];
        return NULL;
      }

      /**
       * Return number of good ASIC windows
       * @return number of good windows
       */
      unsigned getNumofGoodWindows() const {
        unsigned n = 0;
        for (auto & window : m_windows) if (window) n++;
        return n;
      }


    private:

      int m_barID;         /**< bar ID */
      unsigned m_channel;  /**< hardware channel ID */
      float m_timeInterval; /**< time interval between ASIC windows */
      float m_timeAxis[ASICWindowConstants::c_WindowSize]; /**< time axis */
      std::vector<ASICWindowConstants*> m_windows; /**< calib.constants of ASIC windows */

      ClassDef(ASICChannelConstants, 1); /**< ClassDef */

    };

  } // end namespace TOP
} // end namespace Belle2

#endif
