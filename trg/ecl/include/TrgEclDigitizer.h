//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TrgEclDigitizer.h
// Section  : TRG ECL
// Owner    : InSu Lee / Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------------
// Description : A class to represent TRG ECL
//---------------------------------------------------------------
// $Log$
// 2017-02-16 : v01
//---------------------------------------------------------------
#ifndef TRGECLDIGITIZER_H
#define TRGECLDIGITIZER_H

#include <iostream>
#include <TObject.h>
#include <TVector3.h>
#include "trg/ecl/TrgEclMapping.h"
#include "trg/ecl/TrgEclDataBase.h"

namespace Belle2 {

  /*! FAM module   */
  class TrgEclDigitizer : public TObject {

  public:

    /** Constructor */
    TrgEclDigitizer();
    /** Destructor */
    virtual ~TrgEclDigitizer();

    /** setup fam module  */
    void setup(int);
    /** get TC Hits from Xtal hits */
    void getTCHit(int);
    /** fit method,    digi with 125ns interval */
    void digitization01(std::vector<std::vector<double>>&, std::vector<std::vector<double>>&);
    /** orignal no fit method, digi with 12ns interval */
    void digitization02(std::vector<std::vector<double>>&, std::vector<std::vector<double>>&);
    /** save fitting result into tables */
    void save(int);
    /** Set flag of waveform table*/
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
  private:
    /** time range(defult : -4000 ~ 4000 ns ) */
    double TimeRange;
    /** TC Energy converted from Xtarl Energy [GeV] */
    double TCEnergy[576][80];
    /** TC Timing converted from Xtarl Timing [GeV] */
    double TCTiming[576][80];
    /** TC Energy converted from Xtarl Energy [GeV] */
    double TCEnergy_tot[576];
    /** TC Timing converted from Xtarl Timing [GeV] */
    double TCTiming_tot[576];


    /** Input  TC energy[GeV] */
    double TCRawEnergy[576][60];
    /**Input  TC timing[ns]  */
    double TCRawTiming[576][60];
    /**Input  Beambackgroun tag  */
    double TCRawBkgTag[576][60];

    /** Object of TC Mapping */
    TrgEclMapping* _TCMap;
    /** Object of DataBase */
    TrgEclDataBase* _DataBase;



    /** Noise Matrix of Parallel and Serial Noise */
    /** Noise Low triangle Matrix of Parallel noise  */
    std::vector<std::vector<double>> MatrixParallel;
    /** Noise Low triangle Matrix of Serial noise  */
    std::vector<std::vector<double>> MatrixSerial;
    /** Beambackground contribution */
    double TCBkgContribution[576][80];
    /**  Signal contribution */
    double TCSigContribution[576][80];
    /** Beambackground tag */
    int TCBeambkgTag[576][80];
    /** Flag of waveform table */
    int _waveform;
    /** TC Energy converted from Xtarl Energy [GeV] */
    double WaveForm[576][64];


  };
} // end namespace Belle2

#endif
