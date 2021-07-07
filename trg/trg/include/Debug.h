/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGDebug_FLAG_
#define TRGDebug_FLAG_

#include <vector>
#include <string>

namespace Belle2 {

/// A class for debugging of TSIM
  class TRGDebug {

  public:

    /// returns the debug level.
    static int level(void);

    /// sets and returns the debug level.
    static int level(int newLevel, bool thisStageOnly = false);

    /// Declare that you enter new stage.
    static void enterStage(const std::string& stageName);

    /// Declare that you leave a stage.
    static void leaveStage(const std::string& stageName);

    /// returns tab spaces.
    static std::string tab(void);

    /// returns tab spaces with extra spaces
    static std::string tab(int extra);

  private:

    /// Storage for stages.
    static std::vector<std::string> _stages;

    /// Debug level. 0:do nothing, 1:show you flow, 2:show you detail, 10:show you very detail
    static int _level;
  };

//-----------------------------------------------------------------------------

} // namespace Belle2

#endif /* TRGDebug_FLAG_ */
