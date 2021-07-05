/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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

namespace Belle2 {

  class TRGSignalBundle;
  class TRGBoard;

/// A class to represent a serial link between trigger hardware modules
  class TRGChannel {

  public:

    /// Constructor.
    TRGChannel(const std::string& name,
               const TRGBoard& sender,
               const TRGBoard& receiver);

    /// Destructor
    virtual ~TRGChannel();

  public:// Selectors.

    /// returns name.
    const std::string& name(void) const;

    /// returns signal.
    // const TRGSignalBundle * signal(void) const;
    TRGSignalBundle* signal(void) const;

    /// dumps contents. "message" is to select information to
    /// dump. "pre" will be printed in head of each line.
    void dump(const std::string& message = "",
              const std::string& pre = "") const;

    /// sender TRG board
    const TRGBoard& sender(void) const;

  public:// Modifiers.

    /// sets input data.
    // const TRGSignalBundle * signal(const TRGSignalBundle *);
    TRGSignalBundle* signal(TRGSignalBundle*);

  private:

    /// Name of a channel.
    const std::string _name;

    /// Sender.
    const TRGBoard& _sender;

    /// Receiver.
    const TRGBoard& _receiver;

    /// Input data.
    TRGSignalBundle* _data;
  };

//-----------------------------------------------------------------------------

  inline
  const std::string&
  TRGChannel::name(void) const
  {
    return _name;
  }

  inline
  TRGSignalBundle*
  TRGChannel::signal(TRGSignalBundle* a)
  {
    _data = a;
    return _data;
  }

  inline
  TRGSignalBundle*
  TRGChannel::signal(void) const
  {
    return _data;
  }

  inline
  const TRGBoard&
  TRGChannel::sender(void) const
  {
    return _sender;
  }

} // namespace Belle2

#endif /* TRGChannel_FLAG_ */
