/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGECLTIMING_FLAG_
#define TRGECLTIMING_FLAG_

#include <vector>

#include "trg/ecl/TrgEclMapping.h"
//
//
//
namespace Belle2 {
//
//
//
  //
  /*! A Class of  ECL Trigger clustering  */
  //
  class TrgEclTiming {

  public:
    /** Constructor */
    TrgEclTiming();    /// Constructor

    /** Destructor */
    virtual ~TrgEclTiming();/// Destructor

    /** Copy constructor, deleted */
    TrgEclTiming(TrgEclTiming&) = delete;

    /** Assignment operator, deleted */
    TrgEclTiming& operator=(TrgEclTiming&) = delete;

  public:
    //! SetUp
    void Setup(const std::vector<int>&, const std::vector<double>&, const std::vector<double>&);
    //! Get Evnet-timing
    double GetEventTiming(int);//Get eventtming
    //! Fastest TC Timing
    double GetEventTiming00();// Fastest TC timing
    //! The Most energetic TC Timing
    double GetEventTiming01();// The Most energetic TC Timing
    //! Energy weighted Timing of Top 3 energetic TC
    double GetEventTiming02();
    //! Set # of  considered TC  in Energy weighted timing method.
    void SetNofTopTC(int NtopTC) {NofTopTC = NtopTC;}
    //! Get Timing Source
    int GetTimingSource() {return Source;}
    // set energy threshold of event timing quality flag
    void setEventTimingQualityThresholds(const std::vector<double>& iEventTimingQualityThresholds)
    {
      m_EventTimingQualityThresholds = iEventTimingQualityThresholds;
    }
    // get event timing quality flag
    int getEventTimingQualityFlag()
    {
      return m_EventTimingQualityFlag;
    }
    // get TCId which is a source of event timing
    int getEventTimingTCId()
    {
      return m_EventTimingTCId;
    }
    // get theta ID of TC which is a source of event timing
    int getEventTimingTCThetaId()
    {
      return m_EventTimingTCThetaId;
    }
    // get TC energy which is a source of event timing
    double getEventTimingTCEnergy()
    {
      return m_EventTimingTCEnergy;
    }

  private:
    //! TC Energy
    std::vector<double> TCEnergy;
    //! TC Timing
    std::vector<double> TCTiming;
    //! TC Id
    std::vector<int> TCId;

    // Event timing quality flag (for SVD DAQ)
    // 3:= super-fine timing
    // 2:= fine timing
    // 1:= other
    // 0:= no TC hit
    int m_EventTimingQualityFlag;
    // energy threshold(low, high) for quality flag (GeV)
    std::vector<double> m_EventTimingQualityThresholds;
    // TCId of event timing source
    int m_EventTimingTCId;
    // TC ThetaId of event timing source
    int m_EventTimingTCThetaId;
    // TC Energy of event timing source
    double m_EventTimingTCEnergy;

    //! # of  considered TC  in Energy weighted timing method.
    int NofTopTC;
    /** Object of TC Mapping */
    TrgEclMapping* _TCMap;
    //! Timing Source (FWD, Barrel, Backward)
    int Source;
  };
//
//
//
} // namespace Belle2

#endif /* TRGECLTiming_FLAG_ */
