//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Debug.h
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : Debug utility functions
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef TRG_DEBUG

#ifndef TRGDebug_FLAG_
#define TRGDebug_FLAG_

#include <vector>
#include <string>

namespace Belle2 {

/// A class for debugging of TSIM
class TRGDebug {

  public:

    /// Declare that you enter new stage.
    static void enterStage(const std::string & stageName);

    /// Declare that you leave a stage. 
    static void leaveStage(const std::string & stageName);

    /// returns tab spaces.
    static std::string tab(void);

  private:

    /// Storage for stages.
    static std::vector<std::string> _stages;
};

//-----------------------------------------------------------------------------

} // namespace Belle2

#endif /* TRGDebug_FLAG_ */

#endif
