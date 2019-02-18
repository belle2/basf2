//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TrgEclDataBase.h
// Section  : TRG ECL
// Owner    : InSoo Lee / Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------------
// Description : A class to represent TRG ECL
//---------------------------------------------------------------
// $Log$
//---------------------------------------------------------------
#ifndef TRGECLDATABASE_H
#define TRGECLDATABASE_H

#include <iostream>
#include <TObject.h>
#include <TVector3.h>


namespace Belle2 {
  //  class TrgEclDataBase;
  class TrgEclDataBase {

  public:

    /** Constructor */
    TrgEclDataBase();
    /** Destructor */
    virtual ~TrgEclDataBase() {}

  public:
    /**  read coefficient for fit */
    void getCoeffSigPDF(std::vector<std::vector<double>>&, std::vector<std::vector<double>>&);
    /**  read coefficient for fit */
    void getCoeffNoise(int, std::vector<std::vector<double>>&, std::vector<std::vector<double>>&, std::vector<std::vector<double>>&);
    /** Noise Matrix */
    void readNoiseLMatrix(std::vector<std::vector<double>>&, std::vector<std::vector<double>>&);
    /** TC flight time latency  */
    double GetTCFLatency(int);
    /** TC CM Phi  */
    double GetCMPhi(int);
    /** TC CM Phi  */
    double GetCMTheta(int);
    /** TC CM Phi  */
    double GetCMEnergy(int);
    /** TC CM Phi  */
    int Get3DBhabhaLUT(int);
    //! Make Fitter Coefficients from Signal PDF and Noise covariance metrix
    void MakeFitterCoefficient(std::vector<int>, std::vector<int>);
    //! Util to interpalate Signal Shape
    double interFADC(double , std::vector<int>);

  private:
    /** Amplitude Coefficient */
    std::vector<std::vector<int>> _AmpCoefficient;
    /** Timing Coefficient */
    std::vector<std::vector<int>> _TimingCoefficient;



  };
} // end namespace Belle2

#endif
