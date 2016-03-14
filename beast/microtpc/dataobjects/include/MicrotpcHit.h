/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MICROTPCHIT_H
#define MICROTPCHIT_H

#include <simulation/dataobjects/SimHitBase.h>
//#include <framework/datastore/RelationsObject.h>

// ROOT
#include <TObject.h>
#include <TVector3.h>
#include <vector>

namespace Belle2 {
  /**
  * ClassMicrotpcHit - digitization simulated hit for the Microtpc detector.
  *
  * This class holds particle hit data from digitization simulation. As the simulated
  * hit classes are used to generate detector response, they contain _local_
  * information.
  */
  class MicrotpcHit : public SimHitBase {
  public:
    //typedef std::vector<unsigned int>::iterator iterator;
    //typedef std::vector<unsigned int>::const_iterator const_iterator;

    /** default constructor for ROOT */
    MicrotpcHit(): m_column(0), m_row(0), m_BCID(0), m_TOT(0), m_detNb(0) {}


    /** Standard constructor
     * @param energyDep Deposited energy in electrons
     */
    MicrotpcHit(int column, int row, int BCID, int TOT, int detNb):
      m_column(column), m_row(row), m_BCID(BCID), m_TOT(TOT), m_detNb(detNb)
    {
    }

    /** Return the column */
    int getcolumn() const { return m_column; }
    /** Return the row */
    int getrow() const { return m_row; }
    /** Return the BCID */
    int getBCID() const { return m_BCID; }
    /** Return the TOT */
    int getTOT() const { return m_TOT; }
    /** Return the TPC number */
    int getdetNb()  const { return m_detNb; }

  private:

    /** Column */
    int m_column;
    /** Row */
    int m_row;
    /** BCID */
    int m_BCID;
    /** TOT */
    int m_TOT;
    /** Detector Number */
    int m_detNb;

    ClassDef(MicrotpcHit, 1)
  };

} // end namespace Belle2

#endif
