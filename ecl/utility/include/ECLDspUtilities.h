/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov, Mikhail Remnev                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>
#include <ecl/utility/ECLDspEmulator.h>

namespace Belle2 {
  class ECLDigit;
  class ECLDspData;

  namespace ECL {

    /**
     * This struct is returned by the `pedestalFit` method that fits the
     * first 16 samples of the waveform (pedestal points) to find a peak
     * there (that corresponds to background).
     *
     * Currently it only contains peak amplitude but potentially
     * we can add more values here later.
     */
    typedef struct ECLPedestalFit {
      /** Fit amplitude, -128..262015, ADC units (20 ADC units ~= 1 MeV) */
      int amp;
    } ECLPedestalFit;

    /**
     * This class contains static methods to make them
     * accessible from pyROOT.
     */
    class ECLDspUtilities {
    public:
      /**
       * @brief Convert ECLDspData from *.dat file to Root object.
       *
       * @param[in] raw_file Path to dsp??.dat file.
       * @param[in] boardNumber Number of shaperDSP board, from 1 to 52*12
       * @return ECLDspData object
       */
      static ECLDspData* readEclDsp(const char* raw_file, int boardNumber);

      /**
       * @brief Convert ECLDspData from Root object to *.dat file.
       *
       * @param[in] raw_file Path to dsp??.dat file to be created.
       * @param[in] obj      Object to be written
       */
      static void writeEclDsp(const char* raw_file, ECLDspData* obj);

      /**
       * @brief Emulate shape fitting algorithm from ShaperDSP
       *        using algorithm from ecl/utility/src/ECLDspEmulator.cc
       *        See ecl/examples/eclShapeFitter.py for usage example.
       *
       * @param[in] cid      CellID, 1..8736
       * @param[in] adc[31]  Waveform data from ECLDsp dataobject
       * @param[in] ttrig    Trigger time from ECLTrig dataobject
       * @param[in] adjusted_timing Optional.
       *              Use adjusted formula to determine fit time.
       *              Implemented in ShaperDSP firmware since exp 14.
       *              If true, algorithm will determine time near 0 with higher
       *              precision, time of low-energy hits will be one of {-4,0,4}
       *              If false, time will be one of {-32, -16, 0}
       */
      static ECLShapeFit shapeFitter(int cid, std::vector<int> adc, int ttrig,
                                     bool adjusted_timing = true);

      /**
       * Load DSP coefficients used in the pedestal fit function.
       * If it is not done explicitly, pedestalFit will do it
       * internally when it is called the first time.
       *
       * However, it is preferable to call it explicitly, in a
       * thread-safe context.
       */
      static void initPedestalFit();

      /**
       * @brief Fit pedestal part of the signal waveform (first 16 samples)
       * This method will fit the first 16 samples of the waveform
       * and return the amplitude of the peak found in that region.
       *
       * @param[in] adc  vector of waveform samples (size >= 16)
       *
       * @return struct with fit results
       */
      static ECLPedestalFit pedestalFit(std::vector<int> adc);

    private:
      /** Flag indicating whether arrays fg31,fg32 are filled */
      static int pedestal_fit_initialized;
      /** DSP coefficients used to determine amplitude in pedestalFit */
      static float pedfit_fg31[768];
      /** DSP coefficients used to determine time in pedestalFit */
      static float pedfit_fg32[768];

      /**
       * Private constructor since class only contains static methods,
       * no need to create an instance.
       */
      ECLDspUtilities() {}
    };
  }
}

