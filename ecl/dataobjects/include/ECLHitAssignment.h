/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLHITASSIGNMENT_H
#define ECLHITASSIGNMENT_H

#include <framework/datastore/DataStore.h>
#include <framework/datastore/RelationsObject.h>

#include <TVector3.h>

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

#endif
