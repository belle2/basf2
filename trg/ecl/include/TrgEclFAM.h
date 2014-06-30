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

  class TrgEclFAM : public TObject {

  public:

    // Constructor
    TrgEclFAM();

    // Destructor
    virtual ~TrgEclFAM();

    //
    void setup(int, int);
    void getTCHit(int);
    void digitization01(void); // fit method,    digi with 96ns interval
    void digitization02(void); // no fit method, digi with 96ns interval
    void digitization03(void); // orignal no fit method, digi with 12ns interval
    void save(int);

    //! Hit average time
    double m_TimeAve;
    //! The method to set hit average time
    void setTimeAve(double TimeAve) { m_TimeAve = TimeAve; }

    // The method to get TC id
    std::vector<int> TCId(void) const { return _tcid; }
    std::vector<int> TCThetaId(void) const { return _tcthetaid; }
    std::vector<int> TCPhiId(void) const { return _tcphiid; }


    int getTCThetaId(int tcid) const { return _tcthetaid[tcid - 1]; }
    int getTCPhiId(int tcid) const { return _tcphiid[tcid - 1]; }
    int getTCNoOutput(int tcid) const { return _tcnoutput[tcid - 1]; }
    double getTCEnergy(int tcid, int noutput) const {return TCFitEnergy[tcid - 1][noutput]; }
    double getTCTiming(int tcid, int noutput) const {return TCFitTiming[tcid - 1][noutput];}

    // return PDF of shaping based on ShapeF
    double FADC(int, double);
    double SimplifiedFADC(int, double);
    // return function of shaping
    double ShapeF(double, double, double, double, double, double, double);
    double ShapeF(double, double); //Simplified ShapeF
    // return max between two inputs
    double u_max(double, double);
    // read coefficient for fit
    void readFAMDB(void);
    //Noise Matrix
    void readNoiseLMatrix(void);
    // function for fit
    void FAMFit(int,
                int,
                int,
                double,
                double*
               );
    double  GetTCLatency(int);
  private:

    double TimeRange;

    double returnE[100] ;
    double returnT[100] ;
    double TCDigiE[576][80] ;  // [GeV]
    double TCDigiT[576][80] ;  // [ns]



    int noutput[576] ;//The # of output per TC
    int ninput[576]; //The # of input per TC
    double TCEnergy[576][80]; // TC energy[GeV]
    // TC timing[ns] which is weighted by E ( = sum(Ei*Ti)/sum(Ei)).
    double TCTiming[576][80];
    int bin;

    // TC enegry[GeV] and timing for all t=0-8000[ns]
    double TCEnergy_tot[576];
    double TCTiming_tot[576];
    // fitted energy and timing
    double TCFitEnergy[576][60]; // [GeV]
    double TCFitTiming[576][60]; // [ns]

    double TCRawEnergy[576][60]; // Input  TC energy[GeV]
    double TCRawTiming[576][60]; // Input  TC timing[ns]

    std::vector<int>    _tcid;
    std::vector<int>    _tcphiid;
    std::vector<int>    _tcthetaid;
    std::vector<int>    _tcnoutput;
    std::vector<double> _tcenergy;

    TrgEclMapping* _TCMap;

    // coefficient of signal and noise
    double CoeffSigPDF0[192][14];
    double CoeffSigPDF1[192][14];
    double CoeffNoise31[192][14];
    double CoeffNoise32[192][14];
    double CoeffNoise33[192][14];

    // Noise Matrix of Parallel and Serial Noise
    double MatrixParallel[80][80];
    double MatrixSerial[80][80];

  };
} // end namespace Belle2

#endif
