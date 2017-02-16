//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TrgEclFAM.h
// Section  : TRG ECL
// Owner    : InSu Lee / Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------------
// Description : A class to represent TRG ECL
//---------------------------------------------------------------
// $Log$
//---------------------------------------------------------------
#ifndef TRGECLFAM_H
#define TRGECLFAM_H

#include <iostream>
#include <TObject.h>
#include <TVector3.h>
#include "trg/ecl/TrgEclMapping.h"

namespace Belle2 {

  /*! FAM module   */
  class TrgEclFAM : public TObject {

  public:

    /** Constructor */
    TrgEclFAM();

    /** Destructor */
    virtual ~TrgEclFAM();

    /** setup fam module  */
    void setup(int, int);
    /** get TC Hits from Xtal hits */
    void getTCHit(int);
    /** fit method,    digi with 96ns interval */
    void digitization01(void);
    /** no fit method, digi with 96ns interval */
    void digitization02(void);
    /** orignal no fit method, digi with 12ns interval */
    void digitization03(void);
    /** save fitting result into tables */
    void save(int);

    /** Time average */
    double m_TimeAve;
    /** The method to set hit average time */
    void setTimeAve(double TimeAve) { m_TimeAve = TimeAve; }
    void setWaveform(int wave) {_waveform = wave;}


    /** FADC  */
    double FADC(int, double);///FADC
    /** Faster FADC */
    double SimplifiedFADC(int, double);
    /** return shape using  FADC function  */
    double ShapeF(double, double, double, double, double, double, double);
    /** Return shape using Simplified FADC */
    double ShapeF(double, double);
    /** Find max value between 2 vals; */
    double u_max(double, double);
    /**  read coefficient for fit */
    void readFAMDB(void);

    /** Noise Matrix */
    void readNoiseLMatrix(void);
    /** function for fitting  */
    void FAMFit(int,
                int,
                int,
                double,
                double*
               );
    /** TC flight time latency  */
    double  GetTCLatency(int);
  private:
    /** time range(defult : -4000 ~ 4000 ns ) */
    double TimeRange;
    /** Gather TC Energy result  */
    double returnE[100] ;
    /** Gather TC Timing result  */
    double returnT[100] ;
    /** Digitized TC E [GeV] */
    double TCDigiE[576][64];
    /** Digitized TC T [ns] */
    double TCDigiT[576][64];


    /** The # of output per TC */
    int noutput[576] ;
    /** The # of input per TC */
    int ninput[576];
    /** TC energy[GeV] */
    double TCEnergy[576][80];
    /** TC timing[ns] which is weighted by E ( = sum(Ei*Ti)/sum(Ei)). */
    double TCTiming[576][80];
    /** bin */
    // int bin;///bin
    /** TC enegry[GeV] and timing for all t=-4000~4000[ns]*/
    double TCEnergy_tot[576];
    /** TC enegry[GeV] and timing for all t=-4000~4000[ns] */
    double TCTiming_tot[576];

    /** fitted energy and timing */
    double TCFitEnergy[576][60];
    /** */
    double TCFitTiming[576][60];
    /** Input  TC energy[GeV] */
    double TCRawEnergy[576][60];
    /**Input  TC timing[ns]  */
    double TCRawTiming[576][60];
    /**Input  Beambackgroun tag  */
    double TCRawBkgTag[576][60];



    /** Object of TC Mapping */
    TrgEclMapping* _TCMap;


    /** Coeffisients of signal PDF0  */
    std::vector<std::vector<double>> CoeffSigPDF0;
    /** Coeffisients of signal PDF1 */
    std::vector<std::vector<double>> CoeffSigPDF1;
    /** Coeffisients of noise 1 */
    std::vector<std::vector<double>> CoeffNoise31;
    /** Coeffisient of noise 2 */
    std::vector<std::vector<double>> CoeffNoise32;
    /** Coeffisient of noise 3   */
    std::vector<std::vector<double>> CoeffNoise33;

    /** Noise Matrix of Parallel and Serial Noise */
    /**  Noise Low triangle Matrix of Parallel noise  */
    std::vector<std::vector<double>> MatrixParallel;
    /** Noise Low triangle Matrix of Serial noise  */
    std::vector<std::vector<double>> MatrixSerial;
    /** Beambackground contribution */
    double TCBkgContribution[576][80];
    /**  Signal contribution */
    double TCSigContribution[576][80];
    /** Beambackground tag */
    int TCBeambkgTag[576][80];


    int _waveform;


  };
} // end namespace Belle2

#endif
