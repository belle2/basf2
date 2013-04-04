//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : FrontEnd.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a CDC front-end board
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCFrontEnd_FLAG_
#define TRGCDCFrontEnd_FLAG_

#include <fstream>
#include <string>
#include "trg/trg/Board.h"
#include "trg/trg/SignalVector.h"
// #include "trg/trg/OptLink.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCFrontEnd TRGCDCFrontEnd
#endif

namespace Belle2 {

class TRGCDCWire;

/// A class to represent a CDC front-end board
class TRGCDCFrontEnd
    : public TRGBoard,
      public std::vector<const TRGCDCWire *> {
    
  public:// enum
    enum boardType {
        inner = 0,
        outer = 1,
        unknown = 999
    };

  public:

    /// Constructor.
    TRGCDCFrontEnd(const std::string & name, const TRGClock &);

    /// Destructor
    virtual ~TRGCDCFrontEnd();

  public:// Selectors.

    /// returns version.
    static std::string version(void);

    /// returns type.
    boardType type(void) const;

  public:// VHDL utilities

    /// make a VHDL component file. Non-zero value will be returned if
    /// errors occured.
    static int implementation(const boardType & type, std::ofstream &);

    /// writes a port map.
    static int implementationPort(const boardType & type, std::ofstream &);

  public:// Configuration

    void push_back(const TRGCDCWire *);

//     /// dumps contents. "message" is to select information to
//     dump. "pre" will be printed in head of each line.
    void dump(const std::string & message = "",
	      const std::string & pre = "") const;

  private:

    /// Input signal vector.
    TRGSignalVector _input;

};

//-----------------------------------------------------------------------------

} // namespace Belle2

#endif /* TRGCDCFrontEnd_FLAG_ */
