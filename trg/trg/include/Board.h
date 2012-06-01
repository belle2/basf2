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
class TRGLink;

/// A class to represent a trigger board
class TRGBoard {

  public:

    /// Constructor.
    TRGBoard(const std::string & name, const TRGClock &);

    /// Destructor
    virtual ~TRGBoard();

  public:// Selectors.

    /// returns name.
    const std::string & name(void) const;

    /// returns clock.
    const TRGClock & clock(void) const;

    /// dumps contents. "message" is to select information to
    /// dump. "pre" will be printed in head of each line.
    void dump(const std::string & message = "",
              const std::string & pre = "") const;

  public:// Modifiers

    /// appends a link.
    void append(const TRGLink *);

  private:

    /// Name of a board.
    const std::string _name;

    /// Used clock. 
    const TRGClock * _clock;

    /// Serial links.
    std::vector<const TRGLink *> _links;
};

//-----------------------------------------------------------------------------

inline
const std::string &
TRGBoard::name(void) const {
    return _name;
}

inline
const TRGClock &
TRGBoard:: clock(void) const {
    return * _clock;
}

inline
void
TRGBoard::append(const TRGLink * a) {
    _links.push_back(a);
}

} // namespace Belle2

#endif /* TRGBoard_FLAG_ */
