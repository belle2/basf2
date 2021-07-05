/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGECLTIMING_H
#define TRGECLTIMING_H

#include <TObject.h>

namespace Belle2 {

  //!  Digitize result
  class TRGECLTiming : public TObject {
  public:

    //! Event Id
    int m_eventid;
    //! Fitting timing
    double m_timing;
    //! Set event id
    void setEventId(int eventid) { m_eventid = eventid; }
    //! Set fitting timing
    void setTiming(double timing) { m_timing = timing; }


    //! Get event id
    int getEventId() const { return m_eventid; }
    //! Get fit timing
    double getTiming() const {return m_timing; }


    //! Empty constructor
    //! Recommended for ROOT IO
    TRGECLTiming()
    {
      m_eventid = 0;
      m_timing = 0;
    }
    //! the class title
    ClassDef(TRGECLTiming, 1); /*< the class title */
  };
} //! End namespace Belle2

#endif
