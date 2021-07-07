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

#ifndef TRGCDCFrontEnd_FLAG_
#define TRGCDCFrontEnd_FLAG_

#include <fstream>
#include <string>
#include "trg/trg/Board.h"
#include "trg/trg/SignalVector.h"
#include "trg/trg/SignalBundle.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCFrontEnd TRGCDCFrontEnd
#endif

namespace Belle2 {

  class TRGCDCWire;

/// A class to represent a CDC front-end board
  class TRGCDCFrontEnd
    : public TRGBoard,
      public std::vector<const TRGCDCWire*> {

  public:// enum
    /// enum boardType
    enum boardType {
      innerInside  = 0,       // inner of the inner-most super layer
      innerOutside = 1,       // outer of the inner-most super layer
      outerInside  = 2,       // inner of the ouer super layer
      outerOutside = 3,       // outer of the ouer super layer
      unknown = 999
    };

  public:

    /// Constructor.
    TRGCDCFrontEnd(const std::string& name,
                   boardType type,
                   const TRGClock& systemClock,
                   const TRGClock& dataClock,
                   const TRGClock& userClock);

    /// Destructor
    virtual ~TRGCDCFrontEnd();


  public:// Selectors

    /// returns version.
    static std::string version(void);

    /// returns type.
    boardType type(void) const;

  public:// Modifiers

    /// simulates firmware.
    void simulate(void);

    /// output from FrontEnd
    //    TRGSignalBundle  output(void) { return *_osb;};
    TRGSignalBundle* getOSB(void) const { return _osb; };


  public:// VHDL utilities

    /// Makes bit pattern using input bit pattern for the inner FE.
    static TRGState packerInnerInside(const TRGState& input);

    /// Makes bit pattern using input bit pattern for the outer FE.
    static TRGState packerInnerOutside(const TRGState& input);

    /// Makes bit pattern using input bit pattern for the inner FE.
    static TRGState packerOuterInside(const TRGState& input);

    /// Makes bit pattern using input bit pattern for the outer FE.
    static TRGState packerOuterOutside(const TRGState& input);

    /// Unpacks TRGState.
    static void unpackerInnerInside(const TRGState& input,
                                    const TRGState& output);

    /// Unpacks TRGState.
    static void unpackerInnerOutside(const TRGState& input,
                                     const TRGState& output);

    /// Unpacks TRGState.
    static void unpackerOuterInside(const TRGState& input,
                                    const TRGState& output);

    /// Unpacks TRGState.
    static void unpackerOuterOutside(const TRGState& input,
                                     const TRGState& output);

    /// make a VHDL component file. Non-zero value will be returned if
    /// errors occured.
    static int implementation(const boardType& type, std::ofstream&);

    /// writes a port map.
    static int implementationPort(const boardType& type, std::ofstream&);

  public:// Configuration

    /// push back TRGCDCWire for this Front-end
    void push_back(const TRGCDCWire*);

    /// dumps contents. "message" is to select information to
    /// dump. "pre" will be printed in head of each line.
    void dump(const std::string& message = "",
              const std::string& pre = "") const;

    /// Dump all the details of _mosb into a .log file, do it in the end of simulate()
    void dump_log(void) const;
    /// Dump all the details of _mosb into a .log file, for innerInside FE
    void dump_log_innerInside(void) const;
    /// Dump all the details of _mosb into a .log file, for innerOutside FE
    void dump_log_innerOutside(void) const;
    /// Dump all the details of _mosb into a .log file, for outerInside FE
    void dump_log_outerInside(void) const;
    /// Dump all the details of _mosb into a .log file, for outerOutside FE
    void dump_log_outerOutside(void) const;

//  public: // allow Merger class to access FrontEnd data

    //friend class TRGCDCMerger;

  private:

    /// Board type.
    boardType _type;

    /// Input signal bundle.
    TRGSignalBundle* _isb;

    /// Output signal bundle.
    TRGSignalBundle* _osb;
  };

//-----------------------------------------------------------------------------

} // namespace Belle2

#endif /* TRGCDCFrontEnd_FLAG_ */
