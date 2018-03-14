/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/dataobjects/TOPSimHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRawDigit.h>
#include <top/dataobjects/TOPRawWaveform.h>
#include <framework/datastore/StoreArray.h>
#include <top/dbobjects/TOPSampleTimes.h>
#include <top/dbobjects/TOPSignalShape.h>

#include <map>
#include <vector>

namespace Belle2 {
  namespace TOP {
    /**
     * Time digitization of simulated hits in a single electronic channel
     */
    class TimeDigitizer {

    public:

      /**
       * hit type enumerators
       */
      enum EType {
        c_Hit = 0,
        c_ChargeShare = 1,
        c_CrossTalk = 2,
        c_CalPulse = 3
      };

      /**
       * hit data other than time
       */
      struct Hit {
        double pulseHeight = 0; /**< generated pulse height */
        EType type = c_Hit;     /**< hit type */
        const TOPSimHit* simHit = 0;  /**< pointer to simulated hit */
        const TOPSignalShape* shape = 0; /**< waveform shape of this hit */
      };

      /**
       * Constructor
       * @param moduleID TOP module ID
       * @param pixelID pixel ID
       * @param window storage window number (first window of waveform)
       * @param storageDepth analog storage depth
       * @param rmsNoise r.m.s of noise [ADC counts]
       * @param sampleTimes sample times
       */
      TimeDigitizer(int moduleID, int pixelID, unsigned window, unsigned storageDepth,
                    double rmsNoise, const TOPSampleTimes& sampleTimes);

      /**
       * Sets sample times
       * @param sampleTimes sample times
       */
      void setSampleTimes(const TOPSampleTimes* sampleTimes)
      {
        if (sampleTimes) {
          m_sampleTimes = sampleTimes;
        } else {
          B2ERROR("TOP::TimeDigitizer::setSampleTimes: argument is NULL pointer");
        }
      }

      /**
       * Sets noise level
       * @param rmsNoise r.m.s. of noise [ADC counts]
       */
      void setNoise(double rmsNoise) {m_rmsNoise = rmsNoise;}

      /**
       * Add time of simulated hit
       * @param t time of simulated hit
       * @param pulseHeight simulated pulse height
       * @param type hit type
       * @param simHit pointer to simulated hit
       */
      void addTimeOfHit(double t, double pulseHeight, EType type,
                        const TOPSimHit* simHit = 0);

      /**
       * Return bar ID
       * @return bar ID
       */
      int getModuleID() const { return m_moduleID; }

      /**
       * Return pixel ID
       * @return pixel ID (e.g. software channel)
       */
      int getPixelID() const { return m_pixelID; }

      /**
       * Return unique pixel ID
       * @return unique pixel ID
       */
      unsigned getUniqueID() const {return m_pixelID + (m_moduleID << 16);}

      /**
       * Returns ASIC storage window number
       * @return window number
       */
      unsigned getASICWindow() const {return m_window;}

      /**
       * Returns hardware channel number
       * @return hardware channel number
       */
      unsigned int getChannel() const { return m_channel; }

      /**
       * Returns SCROD ID
       * @return SCROD ID
       */
      unsigned getScrodID() const {return m_scrodID;}

      /**
       * Returns carrier board number
       * @return carrier board number
       */
      unsigned getCarrierNumber() const {return m_carrier;}

      /**
       * Returns ASIC number
       * @return ASIC number
       */
      unsigned getASICNumber() const {return m_asic;}

      /**
       * Returns ASIC channel number
       * @return ASIC channel number
       */
      unsigned getASICChannel() const {return m_chan;}

      /**
       * Check if digitizer instance is valid (e.g. module/pixel is mapped to hardware)
       * @return true if valid
       */
      bool isValid() const {return m_valid;}

      /**
       * Do time digitization using simplified pile-up and double-hit-resolution model.
       * As a result, the digitized hits are appended to TOPRawDigits, then
       * they are converted to TOPDigits and the relations to
       * TOPSimHits and MCParticles are set with proper weights.
       *
       * @param rawDigits array of TOPRawDigits
       * @param digits array of TOPDigits
       * @param threshold pulse height threshold [ADC counts]
       * @param thresholdCount minimal number of samples above threshold
       * @param timeJitter a r.m.s. of an additional time jitter due to electronics
       */
      void digitize(StoreArray<TOPRawDigit>& rawDigits,
                    StoreArray<TOPDigit>& digits,
                    int threshold = 0,
                    int thresholdCount = 0,
                    double timeJitter = 0) const;

      /**
       * Do full waveform time digitization.
       * As a result, the digitized hits are appended to TOPRawDigits, then
       * they are converted to TOPDigits and the relations to
       * TOPSimHits and MCParticles are set with proper weights.
       *
       * @param waveforms generated waveforms
       * @param rawDigits array of TOPRawDigits
       * @param digits array of TOPDigits
       * @param threshold pulse height threshold [ADC counts]
       * @param hysteresis pulse height threshold hysteresis [ADC counts]
       * @param thresholdCount minimal number of samples above threshold
       */
      void digitize(StoreArray<TOPRawWaveform>& waveforms,
                    StoreArray<TOPRawDigit>& rawDigits,
                    StoreArray<TOPDigit>& digits,
                    int threshold,
                    int hysteresis = 0,
                    int thresholdCount = 0) const;

    private:

      /**
       * Gauss function (pulse shape approximation)
       * @param x argument
       * @param mean mean
       * @param sigma sigma
       * @return value
       */
      double gauss(double x, double mean, double sigma) const
      {
        double xx = (x - mean) / sigma;
        return exp(-0.5 * xx * xx);
      }

      /**
       * Generate waveform.
       * The size (number of ASIC windows) is given by the size of first argument.
       * The size of second argument must be the same as the first one.
       * @param baselines possible baseline shifts of ASIC windows
       * @param rmsNoises noise levels (r.m.s) per ASIC window
       * @param pedestals average pedestals per ASIC window
       * @param ADCRange ADC range (2^NumBits)
       * @return generated waveform
       */
      std::vector<short> generateWaveform(const std::vector<double>& baselines,
                                          const std::vector<double>& rmsNoises,
                                          const std::vector<double>& pedestals,
                                          int ADCRange) const;


      int m_moduleID = 0;     /**< module ID (1-based) */
      int m_pixelID = 0;      /**< pixel (e.g. software channel) ID (1-based) */
      unsigned m_window = 0;  /**< storage window number */
      unsigned m_storageDepth = 0;  /**< ASIC analog storage depth */
      double m_rmsNoise = 0;  /**< r.m.s of noise [ADC counts] */
      const TOPSampleTimes* m_sampleTimes = 0; /**< sample times */

      unsigned m_channel = 0; /**< hardware channel number (0-based) */
      unsigned m_scrodID = 0; /**< SCROD ID */
      unsigned m_carrier = 0; /**< carrier board number */
      unsigned m_asic = 0;    /**< ASIC number */
      unsigned m_chan = 0;    /**< ASIC channel number */
      bool m_valid = false;   /**< true, if module/pixel is mapped to hardware */

      std::multimap<double, const Hit> m_times; /**< hits sorted by time */

    };

  } // TOP namespace
} // Belle2 namespace

