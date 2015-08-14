/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen, Guglielmo De Nardo                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLDIGITIZERMODULE_H_
#define ECLDIGITIZERMODULE_H_

#include <framework/core/Module.h>
#include <ecl/dataobjects/ECLWaveformData.h>
#include <vector>
#include <boost/multi_array.hpp>

namespace Belle2 {
  namespace ECL {


    /** The ECLDigitizer module.
     *
     * This module is responsible to digitize all hits found in the ECL from ECLHit
     * First, we simualte the sampling array by waveform and amplitude of hit, and
     * smear this sampling array by corresponding error matrix.
     * We then fit the array as hardware of shaper DSP board to obtain the fit result
     * of amplitude, time and quality.
     * The initial parameters of fit and algorithm are same as hardware.
     * This module outputs two array which are ECLDsp and ECLDigit.

       \correlationdiagram
       ECLHit = graph.data('ECLHit')
       ECLDigit   = graph.data('ECLDigit')
       ECLDsp = graph.data('ECLDsp')

       graph.module('ECLDigitizer', [ECLHit], [ECLDigit,ECLDsp])
       graph.relation(ECLDigitizer, ECLHit)
       graph.relation(ECLDigitizer, ECLDigit)
       graph.relation(ECLDigitizer, ECLDsp)
       \endcorrelationdiagram

     */



    class ECLDigitizerModule : public Module {

    public:

      /** Constructor.
       */
      ECLDigitizerModule();

      /** Destructor.
       */
      ~ECLDigitizerModule();


      /** Initialize variables  */
      virtual void initialize();

      /** Nothing so far.*/
      virtual void beginRun();

      /** Actual digitization of all hits in the ECL.
       *
       *  The digitized hits are written into the DataStore.
       */
      virtual void event();

      /** Nothing so far. */
      virtual void endRun();

      /** Free memory */
      virtual void terminate();

      /** read Shaper-DSP data from root file */
      void readDSPDB();

    private:
      typedef boost::multi_array<double, 2> array2d;

      /** Event number */
      int    m_nEvent;

      std::vector<double> m_ft; // shape function looking up table

      // Lookup Table to get aux matrices used in WF fit algorithm
      std::vector<int> m_funcTable;
      std::vector<int> m_eclWaveformDataTable;
      std::vector<int> m_eclWFAlgoParamsTable;
      std::vector<int> m_eclNoiseDataTable;
      std::vector<ECLWFAlgoParams> m_eclWFAlgoParams;
      std::vector<ECLNoiseData> m_eclNoiseData;

      typedef int int_array_192x16_t[192][16];
      typedef int int_array_24x16_t[24][16];
      typedef short int shortint_array_16_t[16];
      typedef std::pair<unsigned int, unsigned int> uint_pair_t;

      struct fitparams_t {
        int_array_192x16_t f, f1, fg31, fg32, fg33;
        int_array_24x16_t fg41, fg43;
      };

      struct algoparams_t {
        shortint_array_16_t id;
      };

      /* Fit algorihtm parameters shared by group of crystals */
      std::vector<algoparams_t> m_idn;
      std::vector<fitparams_t> m_fitparams;

      void shapeFitterWrapper(const int j, const int* FitA, const int m_ttrig,
                              int& m_lar, int& m_ltr, int& m_lq);

      /** Module parameters */
      bool m_background;
      bool m_calibration;
    };
  }//ECL
}//Belle2

#endif /* ECLDIGITIZERMODULE_H_ */
