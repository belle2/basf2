/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include "trg/ecl/TrgEclMapping.h"
#include "trg/ecl/TrgEclDataBase.h"
#include "ecl/dataobjects/ECLHit.h"
#include "trg/ecl/dataobjects/TRGECLBGTCHit.h"

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
    void digitization01(std::vector<std::vector<double>>&,
                        std::vector<std::vector<double>>&);
    /** original no fit method, digi with 12ns interval */
    void digitization02(std::vector<std::vector<double>>&,
                        std::vector<std::vector<double>>&);
    /** save fitting result into tables */
    void save(int);
    /** Set flag of waveform table*/
    void setWaveform(int wave) { m_SaveTCWaveForm = wave;}
    /** Set flag of waveform table*/
    void setFADC(int fadc) { m_FADC = fadc;}



    /** FADC  */
    double FADC(int, double);///FADC
    /** Faster FADC */
    double SimplifiedFADC(int, double);
    /** Faster FADC using interpolation */
    double interFADC(double);
    /** return shape using  FADC function  */
    double ShapeF(double, double, double, double, double, double, double);
    /** Return shape using Simplified FADC */
    double ShapeF(double, double);
    /** Find max value between 2 vals; */
    double u_max(double, double);

  private:

    /** time range(default : -4000 ~ 4000 ns ) */
    double m_TimeRange;
    /** TC Energy converted from Xtarl Energy [GeV] */
    double m_TCEnergy[576][80];
    /** TC Timing converted from Xtarl Timing [GeV] */
    double m_TCTiming[576][80];
    /** TC Energy converted from Xtarl Energy [GeV] */
    double m_TCEnergy_tot[576];
    /** TC Timing converted from Xtarl Timing [GeV] */
    double m_TCTiming_tot[576];

    // background TC Energy [GeV]
    // double TCBGEnergy[576][80];
    // background TC Timing [GeV]
    // double TCBGTiming[576][80];

    /** Input  TC energy[GeV] */
    double m_TCRawEnergy[576][60];
    /**Input  TC timing[ns]  */
    double m_TCRawTiming[576][60];
    /**Input  Beambackground tag  */
    double m_TCRawBkgTag[576][60];

    /** Object of TC Mapping */
    TrgEclMapping* m_TCMap;
    /** Object of DataBase */
    TrgEclDataBase* m_DataBase;

    /** Noise Matrix of Parallel and Serial Noise */
    /** Noise Low triangle Matrix of Parallel noise  */
    std::vector<std::vector<double>> m_MatrixParallel;
    /** Noise Low triangle Matrix of Serial noise  */
    std::vector<std::vector<double>> m_MatrixSerial;
    /** Beambackground contribution */
    double m_TCBkgContribution[576][80];
    /**  Signal contribution */
    double m_TCSigContribution[576][80];
    /** Beambackground tag */
    int m_TCBeambkgTag[576][80];
    /** Flag of waveform table */
    int m_SaveTCWaveForm;
    /** TC Energy converted from Xtarl Energy [GeV] */
    double m_WaveForm[576][64];
    /** Flag of choosing the method of waveform generation function 0: use simplifiedFADC, 1: use interFADC(interpolation) */
    int m_FADC;
    /** Flag of saving beam background tag  or not*/
    int m_BeambkgTag;
  };
} // end namespace Belle2
