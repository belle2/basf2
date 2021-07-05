/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGGRLMATCH.h
// Section  : TRG GRL
// Owner    : Ke LI / Junhao Yin
// Email    : like@ihep.ac.cn / yinjh@ihep.ac.cn
//-----------------------------------------------------------
// Description : storeArray for TRG GRL, only the match info.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------

#ifndef TRGGRLMATCH_H
#define TRGGRLMATCH_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /// a class for CDC2D-ECL Matching in TRGGRL
  class TRGGRLMATCH : public RelationsObject {
  public:

    /// Constructor
    TRGGRLMATCH(): m_deltar(-999.), m_deltaz(-999.), m_e(-99.) {}
    /// Destructor
    ~TRGGRLMATCH() {}

    /** get deltaR */
    double getDeltaR() const {return m_deltar;}

    /**Get the Delta Z (distance along Z-axis between the ECL cluster and the expected position
    which is calculated from CDC) for CDC track and ECL cluster*/
    double getDeltaZ() const {return m_deltaz;}

    /**Get dphi_d*/
    double get_dphi_d() const {return m_dphi_d;}

    /**Get cluster energy*/
    double get_e() const {return m_e;}

    /** set the Delta R*/
    void setDeltaR(double deltar) { m_deltar = deltar; }

    /** set the Delta Z*/
    void setDeltaZ(double deltaz) { m_deltaz = deltaz; }

    /** set the dphi_d*/
    void set_dphi_d(double dphi_d) { m_dphi_d = dphi_d; }

    /** set the cluster energy*/
    void set_e(double e) { m_e = e; }

  private:


    /**Distance in X-Y plane between the ECL cluster and the expected position
    which is calculated from CDC*/
    double m_deltar;

    /**Distance along Z-axis between the ECL cluster and the expected position
    which is calculated from CDC*/
    double m_deltaz;

    /**Phi angle difference between the ECL cluster and the expected position
    which is calculated from CDC*/
    double m_dphi_d = -999.;

    /**Cluster energy*/
    double m_e = -99.;

    /**! The Class title*/
    ClassDef(TRGGRLMATCH, 2); /*< the class title */

  };

} // end namespace Belle2

#endif
