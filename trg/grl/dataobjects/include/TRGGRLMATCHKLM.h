/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGGRLMATCHKLM_H
#define TRGGRLMATCHKLM_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /// a class for CDC2D-KLM Matching in TRGGRL
  class TRGGRLMATCHKLM : public RelationsObject {
  public:

    /// Constructor
    TRGGRLMATCHKLM(): m_dphi(-999.) {}
    /// Desturctor
    ~TRGGRLMATCHKLM() {}

    /**Get dphi*/
    double get_dphi() const {return m_dphi;}

    /** set the dphi*/
    void set_dphi(double dphi) { m_dphi = dphi; }

    /**Get klm sector id*/
    double get_sector() const {return m_sector;}

    /** set the klm sector id*/
    void set_sector(int sector) { m_sector = sector; }


  private:

    /**Phi angle difference between the 2D track's extrapolated hit on superconucting coil
    and the KLM track's sector*/
    double m_dphi;

    /**KLM sector id used for the matching*/
    double m_sector;

    /**! The Class title*/
    ClassDef(TRGGRLMATCHKLM, 3); /*< the class title */

  };

} // end namespace Belle2

#endif
