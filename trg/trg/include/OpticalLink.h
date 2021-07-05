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
// Filename : TRGOpticalLink.h
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a serial link between trigger modules
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGOpticalLink_FLAG_
#define TRGOpticalLink_FLAG_

#include <string>
#include <vector>

namespace Belle2 {

  class TRGSignal;
  class TRGClock;

/// A class to represent a serial link between trigger modules
  class TRGOpticalLink {

  public:
    /// Constructor.
    TRGOpticalLink(const std::string& name, const TRGClock&);

    /// Destructor
    virtual ~TRGOpticalLink();

  public:// Selectors.

    /// returns name.
    const std::string& name(void) const;

    /// returns clock.
    const TRGClock& clock(void) const;

    /// dumps contents. "message" is to select information to
    /// dump. "pre" will be printed in head of each line.
    void dump(const std::string& message = "",
              const std::string& pre = "") const;

  public:// Modifiers.

    /// appends a signal to header data.
    void appendHeader(const TRGSignal*);

    /// appends a signal to trailer data.
    void appendTrailer(const TRGSignal*);

    /// appends a signal to data.
    void append(const TRGSignal*);

  private:

    /// Name of a link.
    const std::string _name;

    /// Used clock.
    const TRGClock* _clock;

    /// Signal header.
    std::vector<const TRGSignal*> _head;

    /// Signal trailer.
    std::vector<const TRGSignal*> _trail;

    /// Signal storage.
    std::vector<const TRGSignal*> _data;
  };

//-----------------------------------------------------------------------------

  inline
  const std::string&
  TRGOpticalLink::name(void) const
  {
    return _name;
  }

  inline
  const TRGClock&
  TRGOpticalLink::clock(void) const
  {
    return * _clock;
  }

  inline
  void
  TRGOpticalLink::appendHeader(const TRGSignal* a)
  {
    _head.push_back(a);
  }

  inline
  void
  TRGOpticalLink::appendTrailer(const TRGSignal* a)
  {
    _trail.push_back(a);
  }

  inline
  void
  TRGOpticalLink::append(const TRGSignal* a)
  {
    _data.push_back(a);
  }

} // namespace Belle2

#endif /* TRGOpticalLink_FLAG_ */
