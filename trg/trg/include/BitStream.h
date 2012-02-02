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

namespace Belle2 {

/// A class to represent a bit stream
class TRGBitStream {

  public:

    /// Default constructor.
    TRGBitStream();

    /// Constructor of fixed size.
    TRGBitStream(int size, const std::string & name = "unknown");

    /// Copy constructor.
    TRGBitStream(const TRGBitStream &);

    /// Destructor
    virtual ~TRGBitStream();

  public:// Selectors

    /// returns name.
    const std::string & name(void) const;

    /// sets and returns name.
    const std::string & name(const std::string & newName);

    /// dumps contents. "message" is to select information to dump. "pre" will be printed in head of each line.
    void dump(const std::string & message = "",
              const std::string & pre = "") const;

    /// returns size of stream.
    unsigned size(void) const;

  public:// Modifiers

    /// clears contents.
    void clear(void);

    /// appends a bit to a stream.
    void append(bool);

    /// appends a bit to a stream.
    void append(int);

    /// appends a bit to a stream.
    void append(unsigned);

  private:

    /// Name.
    std::string _name;

    /// Bit stream max size.
    unsigned _sizeMax;

    /// Bit stream size.
    unsigned _size;

    /// Bit stream storage.
    std::vector<unsigned * > _stream;
};

//-----------------------------------------------------------------------------

inline
const std::string &
TRGBitStream::name(void) const {
    return _name;
}

inline
const std::string &
TRGBitStream::name(const std::string & newName) {
    return _name = newName;
}

inline
void
TRGBitStream::clear(void) {
    _stream.clear();
}

inline
unsigned
TRGBitStream::size(void) const {
    return _size;
}

} // namespace Belle2

#endif /* TRGBitStream_FLAG_ */
