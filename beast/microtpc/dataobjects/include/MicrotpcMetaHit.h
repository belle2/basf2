/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef MICROTPCMETAHIT_H
#define MICROTPCMETAHIT_H

#include <simulation/dataobjects/SimHitBase.h>

namespace Belle2 {
  /**
  * ClassMicrotpcMetaHit - digitization simulated metahit for the Microtpc detector.
  *
  * This class holds particle metahit meta from digitization simulation. As the simulated
  * metahit classes are used to generate detector response, they contain _local_
  * information.
  */
  class MicrotpcMetaHit : public SimHitBase {
  public:

    /** default constructor for ROOT */
    MicrotpcMetaHit(): m_detNb(0), m_pixNb(0),
      m_ts_nb(0), m_ts_start(), m_ts_stop(),
      m_Temperature(), m_Pressure(), m_Flow(), m_SetFlow(0), m_GetFlow(0),
      m_IHER(0), m_PHER(0), m_tHER(0), m_flagHER(0),
      m_ILER(0), m_PLER(0), m_tLER(0), m_flagLER(0) {}
    /** Meta hit store array */
    MicrotpcMetaHit(int detNb, int pixNb,
                    int ts_nb, const double ts_start[10], const double ts_stop[10],
                    const float Temperature[4], const float Pressure[2], const float Flow[2], float SetFlow, float GetFlow,
                    double IHER, double PHER, double tHER, int flagHER,
                    double ILER, double PLER, double tLER, int flagLER):
      m_detNb(detNb), m_pixNb(pixNb), m_ts_nb(ts_nb), m_SetFlow(SetFlow), m_GetFlow(GetFlow),
      m_IHER(IHER), m_PHER(PHER), m_tHER(tHER), m_flagHER(flagHER),
      m_ILER(ILER), m_PLER(PLER), m_tLER(tLER), m_flagLER(flagLER)
    {
      std::copy(ts_start, ts_start + 10, m_ts_start);
      std::copy(ts_stop, ts_stop + 10, m_ts_stop);
      std::copy(Temperature, Temperature + 4, m_Temperature);
      std::copy(Pressure, Pressure + 2, m_Pressure);
      std::copy(Flow, Flow + 2, m_Flow);
    }

    /** Return the TPC number */
    int getdetNb()  const { return m_detNb; }
    /** Return the pixel number firing */
    int getpixNb()  const { return m_pixNb; }
    /** Return the number of time stamp */
    int getts_nb()  const { return m_ts_nb; }
    /** Return start readout time stamp */
    const double* getts_start()  const { return m_ts_start; }
    /** Return stop readout time stamp */
    const double* getts_stop()  const { return m_ts_stop; }
    /** Return TPC temperature */
    const float* getTemperature() const { return m_Temperature; }
    /** Return TPC pressure */
    const float* getPressure() const { return m_Pressure; }
    /** Return TPC flow */
    const float* getFlow() const { return m_Flow; }
    /** Return TPC slow control set flow */
    float getSetFlow() const { return m_SetFlow; }
    /** Return TPC slow control get flow */
    float getGetFlow() const { return m_GetFlow; }
    /** Return HER current */
    double getIHER() const { return m_IHER; }
    /** Return HER average pressure */
    double getPHER() const { return m_PHER; }
    /** Return HER beam life time */
    double gettHER() const { return m_tHER; }
    /** Return HER injection flag */
    double getflagHER() const { return m_flagHER; }
    /** Return LER current */
    double getILER() const { return m_ILER; }
    /** Return LER average pressure */
    double getPLER() const { return m_PLER; }
    /** Return HER beam life time */
    double gettLER() const { return m_tLER; }
    /** Return HER injection flag */
    double getflagLER() const { return m_flagLER; }

  private:

    /** Detector Number */
    int m_detNb;
    /** Pixel number */
    int m_pixNb;
    /** Time stamp number */
    int m_ts_nb;
    /** Start time stamp */
    double m_ts_start[10];
    /** Stop time stamp */
    double m_ts_stop[10];
    /** Temperature */
    float m_Temperature[4];
    /** Pressure */
    float m_Pressure[2];
    /** Flow */
    float m_Flow[2];
    /** Set flow */
    float m_SetFlow;
    /** Get flow */
    float m_GetFlow;
    /** HER current */
    double m_IHER;
    /** HER pressure */
    double m_PHER;
    /** HER tau */
    double m_tHER;
    /** HER injection flag */
    double m_flagHER;
    /** LER current */
    double m_ILER;
    /** LER pressure */
    double m_PLER;
    /** LER tau */
    double m_tLER;
    /** LER injection flag */
    double m_flagLER;

    ClassDef(MicrotpcMetaHit, 1)
  };

} // end namespace Belle2

#endif
