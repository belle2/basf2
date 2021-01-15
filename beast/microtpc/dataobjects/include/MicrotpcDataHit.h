/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2016  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MICROTPCDATAHIT_H
#define MICROTPCDATAHIT_H

#include <simulation/dataobjects/SimHitBase.h>

namespace Belle2 {
  /**
  * ClassMicrotpcDataHit - digitization simulated datahit for the Microtpc detector.
  *
  * This class holds particle datahit data from digitization simulation. As the simulated
  * datahit classes are used to generate detector response, they contain _local_
  * information.
  */
  class MicrotpcDataHit : public SimHitBase {
  public:
    /** default constructor for ROOT */
    MicrotpcDataHit(): m_column(0), m_row(0), m_BCID(0), m_TOT(0), m_detNb(0) {}
    //MicrotpcDataHit(): m_detNb(0) {}

    /** Standard constructor
     * @param column column vector
     * @param row row vector
     * @param BCID BCID vector
     * @param TOT TOT vector
     * @param detNb the TPC number
     */
    MicrotpcDataHit(int column, int row, int BCID, int TOT, int detNb):
      m_column(column), m_row(row), m_BCID(BCID), m_TOT(TOT), m_detNb(detNb)
    {
    }
    /*
    MicrotpcDataHit(int detNb):
    m_detNb(detNb)
    {
    }
    */
    /** Set energy deposition profile
     * To avoid copying of the entries this will take over the contents of the
     * argument and after the call the argument will be empty.
     *
     * TODO: Normally this would be done with an lvalue reference but we need a
     * dictionary so we have to compile it with cint, might change for ROOT6.
     *
     * @param electronProfile list of energy depositions along the simhit,
     * encoded using ElectronDeposit class. Will be empty after call
     */
    /*
    void setcolumn(std::vector<int>& column) {
    m_column.clear();
    std::swap(m_column, column);
    }
    void setrow(std::vector<int>& row) {
    m_row.clear();
    std::swap(m_row, row);
    }
    void setBCID(std::vector<int>& BCID) {
    m_BCID.clear();
    std::swap(m_BCID, BCID);
    }
    void setTOT(std::vector<int>& TOT) {
    m_TOT.clear();
    std::swap(m_TOT, TOT);
    }
    */
    /** Return column vector */
    int getcolumn() const { return m_column; }
    //std::vector<int > getcolumn() const;
    /** Return row vector */
    int getrow() const { return m_row; }
    //std::vector<int > getrow() const;
    /** Return BCID vector */
    int getBCID() const { return m_BCID; }
    //std::vector<int > getBCID() const;
    /** Return TOT vector */
    int getTOT() const { return m_TOT; }
    //std::vector<int > getTOT() const;
    /** Return the TPC number */
    int getdetNb()  const { return m_detNb; }

  private:

    /** Column */
    int m_column;
    //std::vector<int> m_column;
    /** Row */
    int m_row;
    //std::vector<int> m_row;
    /** BCID */
    int m_BCID;
    //std::vector<int> m_BCID;
    /** TOT */
    int m_TOT;
    //std::vector<int> m_TOT;
    /** Detector Number */
    int m_detNb;

    ClassDef(MicrotpcDataHit, 1)
  };

} // end namespace Belle2

#endif
