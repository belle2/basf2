/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
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


      /** Initialize variables, print info, and start CPU clock. */
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

      /** Stopping of CPU clock.*/
      virtual void terminate();

      /** read Shaper-DSP DataBass */
      void readDSPDB();

      /** Shaper-DSP Output  Map */
      double ShaperDSP(double Ti);

      /** Shaper-DSP Output  Map */
      double ShaperDSP_F(double Ti, float* ss);

      /** read Shaper array */
      void DspSamplingArray(int* n, double* t, double* dt, double* ft, double* ff);

      /** Shaper-DSP Output  basic */
      double  Sv123(double t, double t01, double tb1, double t02, double tb2, double td1, double ts1);

      /** Shaper-DSP Fitter */
      void shapeFitter(short int* id, int* f, int* f1, int* fg41, int* fg43, int* fg31, int* fg32, int* fg33, int* y, int* ttrig,
                       int* n16, int* lar, int* ltr, int* lq);


      /** Shaper-DSP Fitter */
      void shapeFitterD(short int* id, int* f, int* f1, int* fg41, int* fg43, int* fg31, int* fg32, int* fg33, int* y, int* ttrig,
                        int* n16, int* ch, int* lar, int* ltr, int* lq);





    protected:
    private:
      typedef boost::multi_array<double, 2> array2d;

      /** Matrix allocation helper function */
      int** allocateMatrix(unsigned int , unsigned int) const;

      /** deallocation functions */
      void deallocate(std::vector<int**> matrices) const;
      void deallocate(int** pointer) const;

      /** Event number */
      int    m_nEvent;

      /** input trigger time 0-23 (*m_ttrig=delta/T*24;    */
      int m_ttrig ;
      /** number of points before signal *m_n16=16       */
      int m_n16 ;
      /** output results: Amplitude 18-bits          */
      int m_lar ;
      /** Time 12 bits               */
      int m_ltr ;
      /**quality 2 bits              */
      int m_lq ;

      /** array of shape function looking up table      */
      double m_ft[1250] ;

      // Lookup Table to get aux matrices used in WF fit algorithm
      ECLLookupTable m_funcTable;

      /* Fit algorihtm parameters shared by group of crystals */
      std::vector< short int*> m_idn;

      // array of function tabulation
      std::vector<int**> m_f; // int [192][16];
      //array of the df/dt tabulation
      std::vector<int**> m_f1; // int [192][16];
      //array for iteration ampl reconstr.
      std::vector<int**> m_fg31; // int [192][16];
      //array for iteration time reconstr.
      std::vector<int**> m_fg32; // int [192][16];
      //array for iteration pedestal reconstr.
      std::vector<int**> m_fg33; // int [192][16];
      //array for iteration ampl. reconstr. (for fixed t)
      std::vector<int**> m_fg41; // int [24][16];
      //array for iteration ped. reconstr. (for fixed t)
      std::vector<int**> m_fg43; // int [24][16];

      //input array to fit
      int FitA[31];
      /** array for calculation of random noise for FitA[31]      */
      float m_vmat[31][31];
      //** pow for int
      int myPow(int x, int y);

      /** Module parameters */
      bool m_background;
      bool m_calibration;
    };
  }//ECL
}//Belle2

#endif /* ECLDIGITIZERMODULE_H_ */
