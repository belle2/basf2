//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGBoard.h
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a trigger board
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGBoard_FLAG_
#define TRGBoard_FLAG_

#include <string>
#include <vector>

namespace Belle2 {

class TRGClock;
class TRGSignalBundle;
class TRGChannel;

/// A class to represent a trigger board
class TRGBoard {

  public:

    /// Constructor.
    TRGBoard(const std::string & name,
	     const TRGClock & systemClock,
	     const TRGClock & dataClock,
	     const TRGClock & userClockInput,
	     const TRGClock & userClockOutput);

    /// Destructor
    virtual ~TRGBoard();

  public:// Selectors.

    /// returns name.
    const std::string & name(void) const;

    /// returns system clock.
    const TRGClock & clockSystem(void) const;

    /// returns data clock.
    const TRGClock & clockData(void) const;

    /// returns Aurora user clock for input.
    const TRGClock & clockUserInput(void) const;

    /// returns Aurora user clock for output.
    const TRGClock & clockUserOutput(void) const;

    /// dumps contents. "message" is to select information to
    /// dump. "pre" will be printed in head of each line.
    void dump(const std::string & message = "",
              const std::string & pre = "") const;

  public:// Modifiers

    /// appends an input Aurora channel.
    void appendInput(const TRGChannel *);

    /// appends an output Aurora channel.
    void appendOutput(TRGChannel *);

  private:

    /// Name of a board.
    const std::string _name;

    /// System clock. 
    const TRGClock * _clockSystem;

    /// Data clock. 
    const TRGClock * _clockData;

    /// User clock. 
    const TRGClock * _clockUserInput;

    /// User clock. 
    const TRGClock * _clockUserOutput;

    /// Input Aurora channel.
    std::vector<const TRGChannel *> _inputChannels;

    /// Output Aurora channel.
    std::vector<TRGChannel *> _outputChannels;
};

//-----------------------------------------------------------------------------

inline
const std::string &
TRGBoard::name(void) const {
    return _name;
}

inline
const TRGClock &
TRGBoard::clockSystem(void) const {
    return * _clockSystem;
}

inline
const TRGClock &
TRGBoard::clockData(void) const {
    return * _clockData;
}

inline
const TRGClock &
TRGBoard::clockUserInput(void) const {
    return * _clockUserInput;
}

inline
const TRGClock &
TRGBoard::clockUserOutput(void) const {
    return * _clockUserOutput;
}

inline
void
TRGBoard::appendInput(const TRGChannel * a) {
    _inputChannels.push_back(a);
}

inline
void
TRGBoard::appendOutput(TRGChannel * a) {
    _outputChannels.push_back(a);
}

} // namespace Belle2

#endif /* TRGBoard_FLAG_ */
