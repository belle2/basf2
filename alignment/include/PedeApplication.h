/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <alignment/dataobjects/PedeSteering.h>
#include <alignment/PedeResult.h>

#include <string>

namespace Belle2 {
  namespace alignment {
    //! Class interfacing Millepede solver (Pede)
    class PedeApplication {
    public:
      //! Constructor
      PedeApplication() : exitCode(-1), exitMessage("") {}
      //! Run Pede with given steering
      //! @param steering Pede steering object with commands
      bool run(PedeSteering& steering);
      //! Run Pede and return full result with parameter corrections
      //! @param steering Pede steering object with commands
      PedeResult calibrate(PedeSteering& steering);
      //! Was Pede successfull (can the result be used)?
      bool success() {return warnings() <= 1;}
      //! Level of warnings converted to int
      int warnings() const;
      //! True if Pede was aborted (parameters not calculated)
      bool aborted() const {return exitCode >= 10 || exitCode == -1;}
      //! Returns the Pede exit code (from millepede.end file)
      int getExitCode() const {return exitCode;}
      //! Returns the Pede exit message (from millepede.end file)
      std::string getExitMessage() const {return exitMessage;}
      //! Reads the millepede.end file and sets this object retrieved state
      void readEndFile(std::string filename = "millepede.end");
      //! Return Pede revision number
      int revision();

    private:
      //! Pede exit code
      int exitCode;
      //! Pede exit message
      std::string exitMessage;
    };
  }
}
