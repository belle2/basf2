/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 Belle II Collaboration                               *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Markus Prim                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <EvtGenBase/EvtSemiLeptonicFF.hh>

#include <framework/utilities/Utils.h>

namespace Belle2 {

  /***
   * BCL Form Factors.
   */
  class EvtBCLFF : public EvtSemiLeptonicFF {

  public:
    EvtBCLFF(int numarg, double* arglist);

    /**
     * Scalar FF's. Implementation follows arXiv:1509.06938v3.
     *
     * For the scalar FF, the arglist in the constructor should contain 8 expansion parameters:
     * b+_0, b+_1, b+_2, b+_3, b0_0, b0_1, b0_2, b0_3
     *
     * @param parent
     * @param daughter
     * @param t  Momentum transfer, also called q2. q2 = (p_B - p_M)^2
     * @param fpf  f_+(q2).
     * @param f0f  f_0(q2).
     */
    void getscalarff(EvtId parent, EvtId daughter, double t, double, double* fpf, double* f0f);

    /**
     * Vector FF's. Implementation follows arXiv:1503.05534v3.
     * It is assumed that each expansion has three terms (hardcoded). However, this can be easily expanded or
     * generalized. It is not done, because this way we can check if the number of arguments in the decay file is the
     * correct one.
     *
     * For the vector FF, the arglist in the constructor should contain 11 expansion parameters:
     * A0_1, A0_2, A1_0, A1_1, A1_2, A12_0, A12_1, A12_2, V_0, V_1, V_2
     * Nota bene: A0_0 is correlated to A12_0.
     *
     * @param parent
     * @param daughter
     * @param t  Momentum transfer, also called q2. q2 = (p_B - p_M)^2
     * @param a1f  A1(q2)
     * @param a2f  A2(q2)
     * @param vf  V(q2)
     * @param a0f  A0(q2)
     */
    void getvectorff(EvtId parent, EvtId daughter, double t, double, double* a1f, double* a2f, double* vf, double* a0f);

    /// Not Implemented
    void gettensorff(EvtId parent, EvtId daughter, double t, double, double* hf, double* kf, double* bp, double* bm);

    /// Not Implemented
    void getbaryonff(EvtId, EvtId, double, double, double*, double*, double*, double*);

    /// Not Implemented
    void getdiracff(EvtId, EvtId, double, double, double*, double*, double*, double*, double*, double*);

    /// Not Implemented
    void getraritaff(EvtId, EvtId, double, double, double*, double*, double*, double*, double*, double*, double*, double*);

  private:

    int m_numBCLFFCoefficients;
    double m_BCLFFCoefficients[19];

    static constexpr double m_resonance0Minus = 5.279;  //  * Unit.GeV
    static constexpr double m_resonance1Minus = 5.325;  //  * Unit.GeV
    static constexpr double m_resonance1Plus = 5.724;  //  * Unit.GeV
  };

}