//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGTOPUnpackerStore.h
// Section  : TRG TOP
// Owner    : Tong Pang  Vladimir Savinov
// Email    : top16@pitt.edu
//-----------------------------------------------------------
// Description : A class to represent TOP L1 combined timing decision
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------
#ifndef TRGTOPUNPACKERSTORE_H
#define TRGTOPUNPACKERSTORE_H

#include <TObject.h>

#include <vector>

#include <trg/top/dataobjects/TRGTOPSlotTiming.h>

namespace Belle2 {


  //! Example Detector
  class TRGTOPUnpackerStore : public TObject {

  public:

    // Empty constructor
    // Recommended for ROOT IO
    TRGTOPUnpackerStore() :
      m_eventIdL1(intNaN),
      m_eventIdTOP(intNaN),
      m_windowIdTOP(intNaN),
      m_rvcB2L(intNaN),
      m_rvcWindow(intNaN),
      m_rvcTopTimingDecisionPrev(intNaN),
      m_rvcTopTimingDecisionNow(intNaN),
      m_rvcTopTimingDecisionNowGdl(intNaN),
      m_combinedTimingTop(intNaN),
      m_nSlotsCombinedTimingTop(intNaN),
      m_combinedTimingTopResidual(intNaN),
      m_nErrorsMinor(intNaN),
      m_nErrorsMajor(intNaN),
      m_trigType(intNaN),
      m_nHitSum(intNaN),
      m_logLSum(intNaN),
      m_logLVar(intNaN),
      m_timingVar(intNaN)
    {}

    //! A Useful Constructor
    explicit TRGTOPUnpackerStore(
      int eventIdL1
    ) :
      m_eventIdL1(eventIdL1),
      m_eventIdTOP(intNaN),
      m_windowIdTOP(intNaN),
      m_rvcB2L(intNaN),
      m_rvcWindow(intNaN),
      m_rvcTopTimingDecisionPrev(intNaN),
      m_rvcTopTimingDecisionNow(intNaN),
      m_rvcTopTimingDecisionNowGdl(intNaN),
      m_combinedTimingTop(intNaN),
      m_nSlotsCombinedTimingTop(intNaN),
      m_combinedTimingTopResidual(intNaN),
      m_nErrorsMinor(intNaN),
      m_nErrorsMajor(intNaN),
      m_trigType(intNaN),
      m_nHitSum(intNaN),
      m_logLSum(intNaN),
      m_logLVar(intNaN),
      m_timingVar(intNaN)
    {}

    //! Another Useful Constructor
    TRGTOPUnpackerStore(
      int eventIdL1,
      int eventIdTOP,
      int windowIdTOP,
      int rvcB2L,
      int rvcWindow,
      int rvcTopTimingDecisionPrev,
      int rvcTopTimingDecisionNow,
      int rvcTopTimingDecisionNowGdl,
      int combinedTimingTop,
      int nSlotsCombinedTimingTop,
      int combinedTimingTopResidual,
      int nErrorsMinor,
      int nErrorsMajor,
      int trigType,
      int nHitSum,
      int logLSum,
      int logLVar,
      int timingVar
    ) :
      m_eventIdL1(eventIdL1),
      m_eventIdTOP(eventIdTOP),
      m_windowIdTOP(windowIdTOP),
      m_rvcB2L(rvcB2L),
      m_rvcWindow(rvcWindow),
      m_rvcTopTimingDecisionPrev(rvcTopTimingDecisionPrev),
      m_rvcTopTimingDecisionNow(rvcTopTimingDecisionNow),
      m_rvcTopTimingDecisionNowGdl(rvcTopTimingDecisionNowGdl),
      m_combinedTimingTop(combinedTimingTop),
      m_nSlotsCombinedTimingTop(nSlotsCombinedTimingTop),
      m_combinedTimingTopResidual(combinedTimingTopResidual),
      m_nErrorsMinor(nErrorsMinor),
      m_nErrorsMajor(nErrorsMajor),
      m_trigType(trigType),
      m_nHitSum(nHitSum),
      m_logLSum(logLSum),
      m_logLVar(logLVar),
      m_timingVar(timingVar)
    {}

    //! Destructor
    ~TRGTOPUnpackerStore() {}

    int getEventIdL1() const { return m_eventIdL1;}
    int getEventIdTOP() const { return m_eventIdTOP;}
    int getWindowIdTOP() const { return m_windowIdTOP;}
    int getRvcB2L() const { return m_rvcB2L;}
    int getRvcWindow() const { return m_rvcWindow;}
    int getRvcTopTimingDecisionPrev() const { return m_rvcTopTimingDecisionPrev;}
    int getRvcTopTimingDecisionNow() const { return m_rvcTopTimingDecisionNow;}
    int getRvcTopTimingDecisionNowGdl() const { return m_rvcTopTimingDecisionNowGdl;}
    int getCombinedTimingTop() const { return m_combinedTimingTop;}
    int getNSlotsCombinedTimingTop() const { return m_nSlotsCombinedTimingTop;}
    int getCombinedTimingTopResidual() const { return m_combinedTimingTopResidual;}
    int getNErrorsMinor() const { return m_nErrorsMinor;}
    int getNErrorsMajor() const { return m_nErrorsMajor;}
    int getTrigType() const { return m_trigType;}
    int getNHitSum() const { return m_nHitSum;}
    int getLogLSum() const { return m_logLSum;}
    int getLogLVar() const { return m_logLVar;}
    int getTimingVar() const { return m_timingVar;}
    //int get() const { return m_;}

    //Need namespace qualifier because ROOT CINT has troubles otherwise
    const std::vector<Belle2::TRGTOPSlotTiming> getSlotTimingDecisions() { return m_slotTimingDecisions; }

    void setEventIdL1(int eventIdL1);
    void setEventIdTOP(int eventIdTOP);
    void setWindowIdTOP(int windowIdTOP);
    void setRvcB2L(int rvcB2L);
    void setRvcWindow(int rvcWindow);
    void setRvcTopTimingDecisionPrev(int rvcTopTimingDecisionPrev);
    void setRvcTopTimingDecisionNow(int rvcTopTimingDecisionNow);
    void setRvcTopTimingDecisionNowGdl(int rvcTopTimingDecisionNowGdl);
    void setCombinedTimingTop(int combinedTimingTop);
    void setNSlotsCombinedTimingTop(int nSlotsCombinedTimingTop);
    void setCombinedTimingTopResidual(int combinedTimingTopResidual);
    void setNErrorsMinor(int nErrorsMinor);
    void setNErrorsMajor(int nErrorsMajor);
    void setTrigType(int trigType);
    void setNHitSum(int nHitSum);
    void setLogLSum(int logLSum);
    void setLogLVar(int logLVar);
    void setTimingVar(int timingVar);
    //void set(int );

    void setSlotTimingDecision(TRGTOPSlotTiming slotTiming);

  private:

    //! L1 event id
    int m_eventIdL1;
    //! TOP decision's event id
    int m_eventIdTOP;
    //! Window number for TOP decision
    int m_windowIdTOP;
    //! rvc at the time when B2L buffer was sent out
    int m_rvcB2L;
    //! rvc of the current window
    int m_rvcWindow;
    //! rvc of previous TOP timing decision
    int m_rvcTopTimingDecisionPrev;
    //! rvc of the current TOP timing decision
    int m_rvcTopTimingDecisionNow;
    //! rvc of the current TOP timing decision sent to GDL
    int m_rvcTopTimingDecisionNowGdl;
    //! combined t0
    int m_combinedTimingTop;
    //! number of slots used for combined decision
    int m_nSlotsCombinedTimingTop;
    //! t0 residual (between actual and estimated timing decisions)
    int m_combinedTimingTopResidual;
    //! the number of minor errors observed during unpacking
    int m_nErrorsMinor;
    //! the number of major errors observed during unpacking
    int m_nErrorsMajor;
    //! trigger type
    int m_trigType;
    //! N hit sum (over slots)
    int m_nHitSum;
    //! logL sum (over slots)
    int m_logLSum;
    //! logL variance (among slots)
    int m_logLVar;
    //! timing variance (among slots)
    int m_timingVar;

    //! vector of slot-level timing decisions
    std::vector<TRGTOPSlotTiming> m_slotTimingDecisions; /**<  a vector of slot-level TOP timing decisions */

    /** the class title*/
    ClassDef(TRGTOPUnpackerStore, 2);

  };

} //end namespace Belle2

#endif

