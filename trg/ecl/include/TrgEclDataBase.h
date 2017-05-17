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
    TrgEclDataBase() {}
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


  };
} // end namespace Belle2

#endif
