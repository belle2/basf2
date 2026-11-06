/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <array>

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

    /** Normalised FADC signal shape, sampled every 12 ns from 0 to 2928 ns.
     *  Used by interFADC() to interpolate the waveform; the corresponding
     *  sampling times are simply 12 ns * index and so are not tabulated. */
    static constexpr std::array<double, 245> m_SignalAmp = {{
        0, 3.47505e-06, 0.000133173, 0.000939532, 0.00337321, 0.00845977, 0.0170396, 0.0296601,
        0.0465713, 0.0677693, 0.0930545, 0.122086, 0.154429, 0.189595, 0.227068, 0.266331,
        0.306882, 0.348243, 0.389971, 0.431664, 0.472959, 0.513539, 0.553127, 0.59149,
        0.628431, 0.663791, 0.697445, 0.729297, 0.759281, 0.787354, 0.813494, 0.837698,
        0.859982, 0.880372, 0.898908, 0.915639, 0.930622, 0.943919, 0.955598, 0.965731,
        0.97439, 0.98165, 0.987587, 0.992275, 0.995788, 0.9982, 0.999581, 1,
        0.999525, 0.998219, 0.996143, 0.993356, 0.989846, 0.985364, 0.979439, 0.971558,
        0.961304, 0.948421, 0.932809, 0.914509, 0.893664, 0.870494, 0.845267, 0.818279,
        0.789837, 0.76025, 0.729815, 0.698815, 0.667511, 0.636141, 0.604919, 0.574035,
        0.543654, 0.513915, 0.484939, 0.456822, 0.429643, 0.403462, 0.378324, 0.354259,
        0.331286, 0.309412, 0.288633, 0.26894, 0.250316, 0.232736, 0.216174, 0.200597,
        0.185972, 0.172262, 0.159428, 0.147431, 0.136232, 0.12579, 0.116067, 0.107022,
        0.0986191, 0.0908193, 0.0835871, 0.0768874, 0.0706867, 0.0649528, 0.0596551, 0.0547641,
        0.0502523, 0.0460932, 0.042262, 0.0387353, 0.0354909, 0.0325082, 0.0297677, 0.0272511,
        0.0249416, 0.0228232, 0.020881, 0.0191014, 0.0174714, 0.0159792, 0.0146138, 0.0133649,
        0.012223, 0.0111793, 0.0102258, 0.00935504, 0.00856, 0.00783438, 0.00717232, 0.00656842,
        0.00601773, 0.00551567, 0.00505807, 0.00464106, 0.00426114, 0.00391506, 0.00359985, 0.0033128,
        0.00305143, 0.00281346, 0.00259681, 0.00239957, 0.00222002, 0.00205655, 0.00190773, 0.00177223,
        0.00164885, 0.00153648, 0.00143413, 0.00134087, 0.00125589, 0.00117842, 0.00110777, 0.00104332,
        0.000984488, 0.000930766, 0.000881678, 0.000836798, 0.000795734, 0.000758134, 0.000723677, 0.00069207,
        0.000663051, 0.000636377, 0.000611832, 0.000589219, 0.000568358, 0.000549086, 0.000531258, 0.000514738,
        0.000499407, 0.000485156, 0.000471884, 0.000459502, 0.00044793, 0.000437092, 0.000426923, 0.000417363,
        0.000408356, 0.000399853, 0.000391811, 0.000384187, 0.000376946, 0.000370055, 0.000363483, 0.000357203,
        0.000351192, 0.000345426, 0.000339886, 0.000334554, 0.000329413, 0.000324448, 0.000319645, 0.000314993,
        0.000310481, 0.000306098, 0.000301836, 0.000297686, 0.00029364, 0.000289693, 0.000285837, 0.000282068,
        0.00027838, 0.000274768, 0.000271229, 0.000267759, 0.000264354, 0.000261011, 0.000257727, 0.000254499,
        0.000251326, 0.000248204, 0.000245132, 0.000242108, 0.000239131, 0.000236198, 0.000233308, 0.00023046,
        0.000227653, 0.000224885, 0.000222155, 0.000219463, 0.000216807, 0.000214187, 0.000211602, 0.00020905,
        0.000206532, 0.000204046, 0.000201593, 0.00019917, 0.000196779, 0.000194417, 0.000192085, 0.000189782,
        0.000187508, 0.000185262, 0.000183044, 0.000180853, 0.000178689, 0.000176552, 0.000174441, 0.000172355,
        0.000170295, 0.00016826, 0.000166249, 0.000164263, 0.000162301,
      }
    };
  };
} // end namespace Belle2
