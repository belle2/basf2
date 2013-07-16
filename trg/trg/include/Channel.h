//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGChannel.h
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a serial link between trigger
// hardware modules
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGChannel_FLAG_
#define TRGChannel_FLAG_

#include <string>
#include <vector>

namespace Belle2 {

class TRGSignalBundle;
class TRGClock;
class TRGBoard;

/// A class to represent a serial link between trigger hardware modules
class TRGChannel {

  public:

    /// Constructor.
    TRGChannel(const std::string & name,
	       const TRGBoard & sender,
	       const TRGBoard & receiver);

    /// Destructor
    virtual ~TRGChannel();

  public:// Selectors.

    /// returns name.
    const std::string & name(void) const;

    /// dumps contents. "message" is to select information to
    /// dump. "pre" will be printed in head of each line.
    void dump(const std::string & message = "",
              const std::string & pre = "") const;

  public:// Modifiers.

    /// sets input data.
    const TRGSignalBundle * input(const TRGSignalBundle *);

  private:

    /// Name of a channel.
    const std::string _name;

    /// Sender.
    const TRGBoard & _sender;

    /// Receiver.
    const TRGBoard & _receiver;

    /// Input data.
    const TRGSignalBundle * _data;
};

//-----------------------------------------------------------------------------

inline
const std::string &
TRGChannel::name(void) const {
    return _name;
}

inline
const TRGSignalBundle *
TRGChannel::input(const TRGSignalBundle * a) {
    _data = a;
    return _data;
}

} // namespace Belle2

#endif /* TRGChannel_FLAG_ */
