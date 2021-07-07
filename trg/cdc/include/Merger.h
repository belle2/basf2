/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to represent a CDC front-end board
//-----------------------------------------------------------------------------

#ifndef TRGCDCMerger_FLAG_
#define TRGCDCMerger_FLAG_

#include <string>
#include "trg/trg/Board.h"
#include "trg/trg/SignalVector.h"
#include "trg/trg/SignalBundle.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCMerger TRGCDCMerger
#endif

namespace Belle2 {

  class TRGCDCFrontEnd;

/// A class to represent a CDC merger board
  class TRGCDCMerger
    : public TRGBoard,
      public std::vector<const TRGCDCFrontEnd*> {

  public:// enum
    /// enum of unitType
    enum unitType {
      innerType = 0,            // for superlayer 0, connected with FE innerInside and innerOutside
      outerType = 1,            // for superlayer 1-7, connected with FE outerInside and outerOutside
      unknown   = 999
    };


  public:
    /// Constructor.
    /*
    TRGCDCMerger(const std::string & name,
     const TRGClock & systemClock,
     const TRGClock & dataClock,
     const TRGClock & userClockInput,
     const TRGClock & userClockOutput);
    */
    TRGCDCMerger(const std::string& name,
                 unitType type,
                 const TRGClock& systemClock,
                 const TRGClock& dataClock,
                 const TRGClock& userClockInput,
                 const TRGClock& userClockOutput);

    /// Destructor
    virtual ~TRGCDCMerger();

  public:// Selectors.

    /// return version.
    static std::string version(void);

    /// return type.
    unitType type(void) const;

  public:// Modifiers

    /// simulates firmware.
    void simulate(void);

    /// Output signal bundle. not the best way to do this though.
    TRGSignalBundle* mosb = nullptr;

  public:// VHDL utilities

    /// Make bit pattern using input information from inner FEs
    static TRGState packerInner(const TRGState& input);

    /// Make bit pattern using input information from outer FEs
    static TRGState packerOuter(const TRGState& input);

    /// Unpack TRGState.
    static void unpackerInner(const TRGState& input,
                              const TRGState& output);

    /// Unpack TRGState.
    static void unpackerOuter(const TRGState& input,
                              const TRGState& output);

    /// make a VHDL component file. Non-zero value will be returned if
    /// errors occured.
    static int implementation(const unitType& type, std::ofstream&);

    /// writes a port map.
    static int implementationPort(const unitType& type, std::ofstream&);


  public:// Configuration

    //TRGSignalBundle* output(void) { return _mosb;};
//yi  TRGSignalBundle* output(void) const { return _mosb;};

    /// push back TRGCDCFrontEnd of this Merger
    void push_back(const TRGCDCFrontEnd*);

    /// dumps contents. "message" is to select information to dump. "pre" will be printed in head of each line.
    void dump(const std::string& message = "",
              const std::string& pre = "") const;

    /// Dump all the details of _mosb into a .log file, do it in the end of simulate()
    void dump_log(void) const;
    /// dump_log for inner Merger
    void dump_log_inner(void) const;
    /// dump_log for outer Merger
    void dump_log_outer(void) const;



  private:

    /// Unit type.
    unitType _type;

    /// Input single bundle.
    TRGSignalBundle* _misb;

    /// outptu signal bundle
    TRGSignalBundle* _mosb;



  };

//-----------------------------------------------------------------------------

} // namespace Belle2

#endif /* TRGCDCMerger_FLAG_ */
