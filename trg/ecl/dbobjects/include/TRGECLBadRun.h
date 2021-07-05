/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#ifndef TRGECLBADRUN_H
#define TRGECLBADRUN_H

#include <TObject.h>

namespace Belle2 {

  //! Raw TC result nefor digitizing
  class TRGECLBadRun : public TObject {
  public:
    //! Empty constructor
    //! Recommended for ROOT IO
    TRGECLBadRun() : m_BadRunNumber(1)
    {

    }
    //! Construction
    explicit TRGECLBadRun(int BadRunNumber):
      m_BadRunNumber(BadRunNumber)
    {}

    //! Set BadRunNumber
    void setBadRunNumber(int BadRunNumber) { m_BadRunNumber = BadRunNumber; }

    //! Get BadRunNumber
    int getBadRunNumber() const
    { return m_BadRunNumber ; }


    //  private :

    //! Bad Run Number
    int m_BadRunNumber;


    //! the class title
    ClassDef(TRGECLBadRun, 1); /*< the class title */
  };
} //! end namespace Belle2

#endif

