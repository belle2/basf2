/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGTOPCOMBINEDT0DECISION_H
#define TRGTOPCOMBINEDT0DECISION_H

//#pragma once

//#include <TObject.h>
#include <framework/datastore/RelationsObject.h>

#include <vector>
#include <limits>

namespace Belle2 {

  //! Example Detector
  //  class TRGTOPCombinedT0Decision : public TObject {
  class TRGTOPCombinedT0Decision : public RelationsObject {

  public:

    // Empty constructor
    // Recommended for ROOT IO
    TRGTOPCombinedT0Decision() :
      m_combinedT0(0),
      m_rvcNow(0),
      m_combinedT0ClockCycle(0),
      m_combinedT0RVC2GDL(0),
      m_eventNumberL1(0),
      m_rvcL1(0)
    {}

    //! A Useful Constructor
    explicit TRGTOPCombinedT0Decision(
      int combinedT0
    ) :
      m_combinedT0(combinedT0)
    {}

    //! Another Useful Constructor
    TRGTOPCombinedT0Decision(
      int combinedT0,
      int rvcNow,
      int combinedT0ClockCycle,
      int combinedT0RVC2GDL,
      int eventNumberL1,
      int rvcL1
    ) :
      m_combinedT0(combinedT0),
      m_rvcNow(rvcNow),
      m_combinedT0ClockCycle(combinedT0ClockCycle),
      m_combinedT0RVC2GDL(combinedT0RVC2GDL),
      m_eventNumberL1(eventNumberL1),
      m_rvcL1(rvcL1)
    {}


    //! Destructor
    ~TRGTOPCombinedT0Decision() {}

    int getCombinedT0() const { return m_combinedT0; }
    int getRVCNow() const { return m_rvcNow; }
    int getCombinedT0ClockCycle() const { return m_combinedT0ClockCycle; }
    int getCombinedT0RVC2GDL() const { return m_combinedT0RVC2GDL; }
    int getEventNumberL1() const { return m_eventNumberL1; }
    int getRVCL1() const { return m_rvcL1; }
    //int get() const { return m_;}

    void setCombinedT0(int combinedT0);
    void setRVCNow(int rvcNow);
    void setCombinedT0ClockCycle(int combinedT0ClockCycle);
    void setCombinedT0RVC2GDL(int combinedT0RVC2GDL);
    void setEventNumberL1(int eventNumberL1);
    void setRVCL1(int rvcL1);
    //void set(int );

  private:

    //! combined t0 (in ns)
    int m_combinedT0;
    //! rvc at the time when this t0 was posted in FW
    int m_rvcNow;
    //! clock cycle number in b2l buffer when this t0 was posted to b2l
    int m_combinedT0ClockCycle;
    //! rvc at the time when this t0 was supposed to be posted to GDL
    int m_combinedT0RVC2GDL;
    //! L1 event number from header
    int m_eventNumberL1;
    //! rvc at the time L1 was issued (for latency and window width studies)
    int m_rvcL1;

    ClassDef(TRGTOPCombinedT0Decision, 1); /**< ClassDef */

  };

} //end namespace Belle2

#endif
