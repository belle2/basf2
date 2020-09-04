/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2016  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <simulation/dataobjects/SimHitBase.h>

namespace Belle2 {
  /**
  * ClassSADMetaHit - digitization simulated metahit for the SAD detector.
  *
  * This class holds particle metahit meta from digitization simulation. As the simulated
  * metahit classes are used to generate detector response, they contain _local_
  * information.
  */
  class SADMetaHit : public SimHitBase {
  public:




    /** default constructor for ROOT */
    SADMetaHit(): m_inputSAD_ssraw(0), m_inputSAD_sraw(0), m_inputSAD_ss(0), m_inputSAD_s(0),
      m_inputSAD_Lss(0), m_inputSAD_nturn(0),
      m_inputSAD_x(0), m_inputSAD_y(0), m_inputSAD_px(0), m_inputSAD_py(0), m_inputSAD_xraw(0), m_inputSAD_yraw(0),
      m_inputSAD_r(0), m_inputSAD_rr(0), m_inputSAD_dp_over_p0(0), m_inputSAD_E(0), m_inputSAD_rate(0), m_inputSAD_watt(0),
      m_inputSAD_ring(0), m_inputSAD_ring_section(0) {}
    /** Meta hit store array */
    SADMetaHit(double inputSAD_ssraw, double inputSAD_sraw, double inputSAD_ss, double inputSAD_s,
               double inputSAD_Lss, int inputSAD_nturn,
               double inputSAD_x, double inputSAD_y, double inputSAD_px, double inputSAD_py, double inputSAD_xraw, double inputSAD_yraw,
               double inputSAD_r, double inputSAD_rr, double inputSAD_dp_over_p0, double inputSAD_E, double inputSAD_rate, double inputSAD_watt,
               int inputSAD_ring, int inputSAD_ring_section):
      m_inputSAD_ssraw(inputSAD_ssraw), m_inputSAD_sraw(inputSAD_sraw), m_inputSAD_ss(inputSAD_ss), m_inputSAD_s(inputSAD_s),
      m_inputSAD_Lss(inputSAD_Lss), m_inputSAD_nturn(inputSAD_nturn),
      m_inputSAD_x(inputSAD_x), m_inputSAD_y(inputSAD_y), m_inputSAD_px(inputSAD_px), m_inputSAD_py(inputSAD_py),
      m_inputSAD_xraw(inputSAD_xraw), m_inputSAD_yraw(inputSAD_yraw),
      m_inputSAD_r(inputSAD_r), m_inputSAD_rr(inputSAD_rr), m_inputSAD_dp_over_p0(inputSAD_dp_over_p0), m_inputSAD_E(inputSAD_E),
      m_inputSAD_rate(inputSAD_rate), m_inputSAD_watt(inputSAD_watt), m_inputSAD_ring(inputSAD_ring),
      m_inputSAD_ring_section(inputSAD_ring_section)
    {
    }

    /** Return scattered position [m] */
    double getssraw()  const { return m_inputSAD_ssraw; }
    /** Return lost position [m] */
    double getsraw()  const { return m_inputSAD_sraw; }
    /** Return  scattered position (|s|<Ltot/2) [m] */
    double getss()  const { return m_inputSAD_ss; }
    /** Return lost position (|s|<Ltot/2) [m] */
    double gets()  const { return m_inputSAD_s; }
    /** Return length of element in which scattered [m] */
    double getLss()  const { return m_inputSAD_Lss; }
    /** Return number of turns from scattered to lost */
    int getn_turn()  const { return m_inputSAD_nturn; }
    /** Return x-coordinate of the lost particle */
    double getx()  const { return m_inputSAD_x; }
    /** Return y-coordinate of the lost particle  */
    double gety()  const { return m_inputSAD_y; }
    /** Return momentum x-coordinate of the lost particle */
    double getpx()  const { return m_inputSAD_px; }
    /** Return momentum y-coordinate of the lost particle */
    double getpy()  const { return m_inputSAD_py; }
    /** Return  x at lost position [m] before matching onto beam pipe inner surface */
    double getxraw()  const { return m_inputSAD_xraw; }
    /** Return  x at lost position [m] before matching onto beam pipe inner surface */
    double getyraw()  const { return m_inputSAD_yraw; }
    /** Return sqrt(x*x+y*y) [m] */
    double getr()  const { return m_inputSAD_r; }
    /** Return sqrt(x*x+y*y) [m]before matching onto beam pipe inner surface */
    double getrr()  const { return m_inputSAD_rr; }
    /** Return momentum deviation of the lost particle  */
    double getdp_over_p0()  const { return m_inputSAD_dp_over_p0; }
    /** Return energy loss */
    double getE()  const { return m_inputSAD_E; }
    /** Return loss rate [Hz] */
    double getrate()  const { return m_inputSAD_rate; }
    /** Return loss wattage [W] */
    double getwatt()  const { return m_inputSAD_watt; }
    /** Return ring HER = 1 or LER = 2 */
    int getring()  const { return m_inputSAD_ring; }
    /** Return section of ring where the scattering occures */
    int getring_section()  const { return m_inputSAD_ring_section; }

  private:

    /** Return scattered position [m] */
    double m_inputSAD_ssraw;
    /** Return lost position [m] */
    double m_inputSAD_sraw;
    /** Return scattered position (|s|<Ltot/2) [m] */
    double m_inputSAD_ss;
    /** Return lost position (|s|<Ltot/2) [m] */
    double m_inputSAD_s;
    /** Return length of element in which scattered [m] */
    double m_inputSAD_Lss;
    /** Return number of turns from scattered to lost */
    int m_inputSAD_nturn;
    /** Return x-coordinate of the lost particle */
    double m_inputSAD_x;
    /** Return y-coordinate of the lost particle */
    double m_inputSAD_y;
    /** Return  momentum x-coordinate of the lost particle  */
    double m_inputSAD_px;
    /** Return  momentum y-coordinate of the lost particle */
    double m_inputSAD_py;
    /** Return x at lost position [m] before matching onto beam pipe inner surface */
    double m_inputSAD_xraw;
    /** Return y at lost position [m] before matching onto beam pipe inner surface */
    double m_inputSAD_yraw;
    /** Return sqrt(x*x+y*y) [m] */
    double m_inputSAD_r;
    /** Return sqrt(x*x+y*y) [m] at lost position [m] before matching onto beam pipe inner surface */
    double m_inputSAD_rr;
    /** Return dp over p0 */
    double m_inputSAD_dp_over_p0;
    /** Return energy loss */
    double m_inputSAD_E;
    /** Return loss rate [Hz] */
    double m_inputSAD_rate;
    /** Return watt loss [Hz] */
    double m_inputSAD_watt;
    /** Return ring HER = 1 or LER = 2 */
    int m_inputSAD_ring;
    /** Return section of ring where the scattering occures */
    int m_inputSAD_ring_section;

    ClassDef(SADMetaHit, 2)
  };

} // end namespace Belle2

