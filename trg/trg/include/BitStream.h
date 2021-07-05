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
// Filename : TRGBitStream.h
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a bit stream.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGBitStream_FLAG_
#define TRGBitStream_FLAG_

#include <vector>
#include <string>
#include "trg/trg/Signal.h"

//...Data structure of TRGBitStreamFile...
//   4 byte                  : record type
//   4 byte                  : size in bit
//   variable(size / 8) byte : data

//...TRGBitStream record definition...
#define TRGBSRecord_Comment    0xffff0000
#define TRGBSRecord_BeginRun   0xffff00A0
#define TRGBSRecord_EndRun     0xffff00A1
#define TRGBSRecord_BeginEvent 0xffff00B0
#define TRGBSRecord_EndEvent   0xffff00B1
#define TRGBSRecord_Clock      0xffff00C0
#define TRGBSRecord_SegmentSL0 0xffff00F0
#define TRGBSRecord_SegmentSL1 0xffff00F1
#define TRGBSRecord_SegmentSL2 0xffff00F2
#define TRGBSRecord_SegmentSL3 0xffff00F3
#define TRGBSRecord_SegmentSL4 0xffff00F4
#define TRGBSRecord_SegmentSL5 0xffff00F5
#define TRGBSRecord_SegmentSL6 0xffff00F6
#define TRGBSRecord_SegmentSL7 0xffff00F7
#define TRGBSRecord_SegmentSL8 0xffff00F8

namespace Belle2 {

/// A class to represent a bit stream
  class TRGBitStream {

  public:

    /// Default constructor.
    TRGBitStream();

    /// Constructor of fixed size.
    explicit TRGBitStream(int size, const std::string& name = "unknown");

    /// Copy constructor.
    TRGBitStream(const TRGBitStream&);

    /// Constructor with char stream.
    TRGBitStream(const char* const, unsigned sizeInBit);

    /// Destructor
    virtual ~TRGBitStream();

  public:// Selectors

    /// returns name.
    const std::string& name(void) const;

    /// sets and returns name.
    const std::string& name(const std::string& newName);

    /// dumps contents. "message" is to select information to dump. "pre" will be printed in head of each line.
    void dump(const std::string& message = "",
              const std::string& pre = "") const;

    /// returns size of stream in unit of bit.
    unsigned size(void) const;

    /// returns size in char's.
    unsigned sizeInChar(void) const;

    /// returns a pointer to char's.
    char c(unsigned positionInChar) const;

    /// returns true if given position is active.
    bool bit(unsigned positionInBit) const;

  public:// Modifiers

    /// clears contents.
    void clear(void);

    /// appends a bit to a stream.
    void append(bool);

    /// appends a bit to a stream.
    void append(int);

    /// appends a bit to a stream.
    void append(unsigned);

  public:// Utility functions

    /// Make trigger signals from bit stream.
    static std::vector<TRGSignal> TRGBitStream2TRGSignal(
      const TRGClock& clock,
      int initialClockPosition,
      std::vector<TRGBitStream*> stream);

  private:

    /// Name.
    std::string _name;

    /// Bit stream max size.
    unsigned _sizeMax;

    /// Bit stream size.
    unsigned _size;

    /// Bit stream storage.
    std::vector<unsigned* > _stream;
  };

//-----------------------------------------------------------------------------

  inline
  const std::string&
  TRGBitStream::name(void) const
  {
    return _name;
  }

  inline
  const std::string&
  TRGBitStream::name(const std::string& newName)
  {
    return _name = newName;
  }

  inline
  void
  TRGBitStream::clear(void)
  {
    _stream.clear();
  }

  inline
  unsigned
  TRGBitStream::size(void) const
  {
    return _size;
  }

  inline
  void
  TRGBitStream::append(int a)
  {
    if (a)
      append(true);
    else
      append(false);
  }

  inline
  void
  TRGBitStream::append(unsigned a)
  {
    if (a)
      append(true);
    else
      append(false);
  }

  inline
  unsigned
  TRGBitStream::sizeInChar(void) const
  {
    unsigned s = _size / 8;
    if (_size % 8)
      ++s;
    return s;
  }

  inline
  char
  TRGBitStream::c(unsigned a) const
  {
    unsigned p = a / sizeof(unsigned);
    unsigned q = a % sizeof(unsigned);
    unsigned v = * _stream[p];
    return (v >> (q * 8)) & 0xff;
  }

  inline
  bool
  TRGBitStream::bit(unsigned a) const
  {
    unsigned p = a / (sizeof(unsigned) * 8);
    unsigned q = a % (sizeof(unsigned) * 8);
    unsigned v = * _stream[p];
    return (v & (1 << q));
  }

} // namespace Belle2

#endif /* TRGBitStream_FLAG_ */
