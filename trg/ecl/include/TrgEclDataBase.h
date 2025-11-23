/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGECLDATABASE_H
#define TRGECLDATABASE_H

#include <vector>

namespace Belle2 {
  ///  class TrgEclDataBase;
  class TrgEclDataBase {

  public:

    //! TrgEclDataBase Constructor
    TrgEclDataBase();
    //! TrgEclDataBase Destructor
    virtual ~TrgEclDataBase() {}

  public:
    /**  read coefficient for fit */
    void getCoeffSigPDF(std::vector<std::vector<double>>&, std::vector<std::vector<double>>&);
    /**  read coefficient for fit */
    void getCoeffNoise(int, std::vector<std::vector<double>>&, std::vector<std::vector<double>>&, std::vector<std::vector<double>>&);
    /** Noise Matrix */
    void readNoiseLMatrix(std::vector<std::vector<double>>&, std::vector<std::vector<double>>&);
    /** TC flight time latency  */
    double getTCFLatency(int);
    /** TC CM Phi  */
    double getCMPhi(int);
    /** TC CM Phi  */
    double getCMTheta(int);
    /** TC CM Phi  */
    double getCMEnergy(int);
    /** TC CM Phi  */
    int get3DBhabhaLUT(int);
    //! Make Fitter Coefficients from Signal PDF and Noise covariance matrix
    void MakeFitterCoefficient(const std::vector<int>&, std::vector<int>);
    //! Util to interpolate Signal Shape
    double interFADC(double, std::vector<int>);

  private:
    /** Amplitude Coefficient */
    std::vector<std::vector<int>> m_AmpCoefficient;
    /** Timing Coefficient */
    std::vector<std::vector<int>> m_TimingCoefficient;



  };
} // end namespace Belle2

#endif
