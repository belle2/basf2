/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLDIGIMODULE_H_
#define ECLDIGIMODULE_H_

#include <framework/core/Module.h>
#include <vector>
#include <TRandom3.h>

namespace Belle2 {
  namespace ECL {

    /** Module for making ECLRecoHits of CDCHits.
     *
     */
    class ECLDigiModule : public Module {

    public:

      /** Constructor.
       */
      ECLDigiModule();

      /** Destructor.
       */
      ~ECLDigiModule();


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

      /** Shaper-DSP Output  basic */
      double  Sv123(double t, double t01, double tb1, double t02, double tb2, double td1, double ts1);

      /** Shaper-DSP Fitter */
      void shapeFitter(short int* id, int* f, int* f1, int* fg41, int* fg43, int* fg31,
                       int* fg32, int* fg33, int* y, int* ttrig, int* n16, int* ch, int* lar, int* ltr, int* lq);



    protected:
    private:

      /** Input ECLSimHit array name. */
      std::string m_inColName;
      /** Name of collection of ECLHits.
       */
      std::string m_eclHitCollectionName;

      /** Name of output Hit collection of this module.
        */
      std::string m_eclDigiCollectionName;


      /** Name of output ECLShaperArray collection of this module.
        */
      std::string m_eclDspCollectionName;


      /** Name of the  Random Time of Trig record
        */
      std::string m_eclTrigCollectionName;


//    unsigned int m_randSeed;    /**< User-supplied random seed, default is 0. for ctime  */
      /**< Random number generator.*/
      TRandom3* m_random;
      /** CPU time     */
      double m_timeCPU;
      /** Run number   */
      int    m_nRun;
      /** Event number */
      int    m_nEvent;
      /** index of eclDsp */
      int    m_hitNum;
      /** index of Digitized result*/
      int    m_hitNum1;
      /** index of trig*/
      int    m_hitNum2;

      /** input trigger time 0-23 (*m_ttrig=delta/T*24;    */
      int m_ttrig ;
      /** number of points before signal *m_n16=16       */
      int m_n16 ;
      /** channel number for MC m_ch=0           */
      int m_ch ;
      /** output results: Amplitude 18-bits          */
      int m_lar ;
      /** Time 12 bits               */
      int m_ltr ;
      /**quality 2 bits              */
      int m_lq ;

      /** array of shaper dsp settings           */
      short int m_id[16][16];
      /** array of function tabulation                 */
      int m_f[192][16];
      /**array of the df/dt tabulation               */
      int m_f1[192][16];
      /**array for iteration ampl reconstr.                */
      int m_fg31[192][16];
      /**array for iteration time reconstr.                */
      int m_fg32[192][16];
      /**array for iteration pedestal reconstr.              */
      int m_fg33[192][16];
      /**array for iteration ampl. reconstr. (for fixed t)       */
      int m_fg41[24][16];
      /**array for iteration ped. reconstr. (for fixed t)        */
      int m_fg43[24][16];
      /**input array to fit          */
      int FitA[31];
      /** array for calculation of random noise for FitA[31]      */
      double m_vmat[31][31];

    };
  }//ECL
}//Belle2

#endif /* EVTMETAINFO_H_ */
