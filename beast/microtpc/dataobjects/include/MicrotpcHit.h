/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef MICROTPCHIT_H
#define MICROTPCHIT_H

#include <simulation/dataobjects/SimHitBase.h>

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
    //MicrotpcHit(): m_column(0), m_row(0), m_BCID(0), m_TOT(0), m_detNb(0), m_pdg(0) {}
    //MicrotpcHit(): m_column(0), m_row(0), m_BCID(0), m_TOT(0), m_detNb(0) {}
    MicrotpcHit(): m_column(0), m_row(0), m_BCID(0), m_TOT(0), m_detNb(0), m_pdg(0), m_trkID(0) {}

    /** Standard constructor
     * @param column column vector
     * @param row row vector
     * @param BCID BCID vector
     * @param TOT TOT vector
     * @param detNb the TPC number
     * @param pdg the PDG of particles
     * @param trkID the track ID
     */
    /*
    MicrotpcHit(int column, int row, int BCID, int TOT, int detNb, int pdg):
      m_column(column), m_row(row), m_BCID(BCID), m_TOT(TOT), m_detNb(detNb), m_pdg(pdg)
    {
    }
    */
    /*
    MicrotpcHit(int column, int row, int BCID, int TOT, int detNb):
      m_column(column), m_row(row), m_BCID(BCID), m_TOT(TOT), m_detNb(detNb)
    {
    }
    */
    MicrotpcHit(int column, int row, int BCID, int TOT, int detNb, int pdg, int trkID):
      m_column(column), m_row(row), m_BCID(BCID), m_TOT(TOT), m_detNb(detNb), m_pdg(pdg), m_trkID(trkID)
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
    /** Return the PDG of particles */
    int getPDG()  const { return m_pdg; }
    /** Return the track ID */
    int gettrkID()  const { return m_trkID; }

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
    /** Particle PDG */
    int m_pdg;
    /** Particle PDG */
    int m_trkID;

    ClassDef(MicrotpcHit, 1)
  };

} // end namespace Belle2

#endif
