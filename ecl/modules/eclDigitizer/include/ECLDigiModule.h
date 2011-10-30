/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLDIGIMODULE_H_
#define ECLDIGIMODULE_H_

#include <framework/core/Module.h>
#include <vector>
#include <TRandom3.h>

namespace Belle2 {

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

    //** read Shaper-DSP DataBass
    void readDSPDB();

    //** Shaper-DSP Output  Map
    double ShaperDSP(double Ti);

    //** Shaper-DSP Output  basic
    double  Sv123(double t, double t01, double tb1, double t02, double tb2, double td1, double ts1);

    void shapeFitter(short int *id, int *f, int *f1, int *fg41, int *fg43, int *fg31,
                     int *fg32, int *fg33, int *y, int *ttrig, int *n16, int *ch, int *lar, int *ltr, int *lq);



  protected:


    void printModuleParams() const;

  private:
    /** Name of collection of ECLHits.
     */
    std::string m_eclHitCollectionName;

    /** Name of output Hit collection of this module.
      */
    std::string m_eclDigiCollectionName;


    /** Name of output ECLShaperArray collection of this module.
      */
    std::string m_eclDspCollectionName;

//    unsigned int m_randSeed;    /**< User-supplied random seed, default is 0. for ctime  */
    TRandom3* m_random;     /**< Random number generator.*/

    double m_timeCPU;                /*!< CPU time     */
    int    m_nRun;                   /*!< Run number   */
    int    m_nEvent;                 /*!< Event number */
    int    m_hitNum;

    int m_ttrig ;        /*! input trigger time 0-23 (*m_ttrig=delta/T*24;    */
    int m_n16 ;          /*! number of points before signal *m_n16=16       */
    int m_ch ;           /*! channel number for MC m_ch=0           */
    int m_lar ;          /*! output results: Amplitude 18-bits          */
    int m_ltr ;          /*! Time 12 bits               */
    int m_lq ;           /*!quality 2 bits              */


    short int m_id[16][16];      /*! array of shaper dsp settings           */
    int m_f[192][16];        /*! array of function tabulation                 */
    int m_f1[192][16];         /*!array of the df/dt tabulation               */
    int m_fg31[192][16];       /*!array for iteration ampl reconstr.                */
    int m_fg32[192][16];       /*!array for iteration time reconstr.                */
    int m_fg33[192][16];       /*!array for iteration pedestal reconstr.              */

    int m_fg41[24][16];        /*!array for iteration ampl. reconstr. (for fixed t)       */
    int m_fg43[24][16];        /*!array for iteration ped. reconstr. (for fixed t)        */
    int FitA[31];        /*!input array to fit          */

    double m_vmat[31][31];       /* array for calculation of random noise for FitA[31]      */

  };
}

#endif /* EVTMETAINFO_H_ */
