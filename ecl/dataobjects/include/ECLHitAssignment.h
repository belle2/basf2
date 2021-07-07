/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  //! Example Detector
  class ECLHitAssignment : public RelationsObject {
  public:

    //! The method to set shower id
    void setShowerId(int showerId) { m_showerId = showerId; }

    //! The method to set cell id
    void setCellId(int cellId) { m_cellId = cellId; }

    //! The method to get shower id
    int getShowerId() const { return m_showerId; }

    //! The method to get cell id
    int getCellId() const { return m_cellId; }

    //! Empty constructor
    /*! Recommended for ROOT IO
    */
    ECLHitAssignment()
    {
      m_showerId = 0;
      m_cellId = 0;
      ;
    }

    //! Useful Constructor
    ECLHitAssignment(
      int showerId,
      int cellId
    )
    {
      m_showerId = showerId;
      m_cellId = cellId;
    }

    ClassDef(ECLHitAssignment, 1);/**< the class title */

  private:

    //! The cell id of this hit.
    int m_showerId;

    //! The cell id of this hit.
    int m_cellId;
  };

} // end namespace Belle2

