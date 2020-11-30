/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2016  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MICROTPCMETAEDATAHIT_H
#define MICROTPCMETAEDATAHIT_H

#include <simulation/dataobjects/SimHitBase.h>
//#include <framework/datastore/RelationsObject.h>

// ROOT
#include <TVector3.h>

namespace Belle2 {
  /**
  * ClassMicrotpcDataHit - digitization simulated datahit for the Microtpc detector.
  *
  * This class holds particle datahit data from digitization simulation. As the simulated
  * datahit classes are used to generate detector response, they contain _local_
  * information.
  */
  class MicrotpcMetaEDataHit : public SimHitBase {
  public:
    /** default constructor for ROOT */
    //MicrotpcMetaEDataHit(): m_column(0), m_row(0), m_BCID(0), m_TOT(0), m_detNb(0) {}
    MicrotpcMetaEDataHit(): m_detNb(0), m_pixNb(0),
      m_ts_nb(0), m_ts_start(), m_ts_stop(),
      m_Temperature(), m_Pressure(), m_Flow(), m_SetFlow(0), m_GetFlow(0),
      m_IHER(0), m_PHER(0), m_tHER(0), m_flagHER(0),
      m_ILER(0), m_PLER(0), m_tLER(0), m_flagLER(0) {}

    /** Standard constructor
     * @param detNb the TPC number
     * @param pixNb the pixel number firing
     * @param ts_nb the number of time stamp
     * @param ts_start start readout time stamp
     * @param ts_stop stop readout time stamp
     * @param Temperature TPC temperature
     * @param Pressure TPC pressure
     * @param Flow TPC flow
     * @param SetFlow TPC slow control set flow
     * @param GetFlow TPC slow control get flow
     * @param IHER HER current
     * @param PHER HER average pressure
     * @param tHER HER beam life time
     * @param flagHER HER injection flag
     * @param ILER LER current
     * @param PLER LER average pressure
     * @param tLER LER beam life time
     * @param flagLER LER injection flag
     */
    /*
    MicrotpcMetaEDataHit(int column, int row, int BCID, int TOT, int detNb):
    m_column(column), m_row(row), m_BCID(BCID), m_TOT(TOT), m_detNb(detNb)
    {
    }
    */
    MicrotpcMetaEDataHit(int detNb, int pixNb,
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


    /** Set column into a vector array */
    void setcolumn(std::vector<int>& column)
    {
      m_column.clear();
      std::swap(m_column, column);
    }
    /** Set row into a vector array */
    void setrow(std::vector<int>& row)
    {
      m_row.clear();
      std::swap(m_row, row);
    }
    /** Set bc into a vector array */
    void setBCID(std::vector<int>& BCID)
    {
      m_BCID.clear();
      std::swap(m_BCID, BCID);
    }
    /** Set tot into a vector array */
    void setTOT(std::vector<int>& TOT)
    {
      m_TOT.clear();
      std::swap(m_TOT, TOT);
    }

    /** Return column vector */
    //int getcolumn() const { return m_column; }
    std::vector<int > getcolumn() const;
    /** Return row vector */
    //int getrow() const { return m_row; }
    std::vector<int > getrow() const;
    /** Return BCID vector */
    //int getBCID() const { return m_BCID; }
    std::vector<int > getBCID() const;
    /** Return TOT vector */
    //int getTOT() const { return m_TOT; }
    std::vector<int > getTOT() const;
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

    /** Column */
    //int m_column;
    std::vector<int> m_column;
    /** Row */
    //int m_row;
    std::vector<int> m_row;
    /** BCID */
    //int m_BCID;
    std::vector<int> m_BCID;
    /** TOT */
    //int m_TOT;
    std::vector<int> m_TOT;
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

    ClassDef(MicrotpcMetaEDataHit, 1)
  };

} // end namespace Belle2

#endif
