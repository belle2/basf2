/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <alignment/PedeApplication.h>

#include <fstream>
#include <sstream>

using namespace std;
namespace Belle2 {
  namespace alignment {

    // -----------
    // PedeApplication
    // -----------
    int PedeApplication::revision()
    {
      int ver(-1);
      string tmpfile("pede_test.tmp.txt");

      string cmd("pede > " + tmpfile);
      system(cmd.c_str());

      ifstream file(tmpfile);
      string line; string verText;

      getline(file, line);
      stringstream ss(line);
      ss >> verText >> ver;

      remove(tmpfile.c_str());
      return ver;
    }
    void PedeApplication::readEndFile(string filename)
    {
      stringstream ss;
      ifstream result(filename);
      result >> exitCode;
      ss << result.rdbuf();
      exitMessage = ss.str();
    }
    PedeResult PedeApplication::calibrate(PedeSteering& steering)
    {
      PedeResult result;
      if (run(steering)) {
        result.read("millepede.res");
        result.readEigenFile("millepede.eve");
      }
      return result;
    }
    bool PedeApplication::run(PedeSteering& steering)
    {
      string cmd("pede " + steering.make());
      int retval = system(cmd.c_str());
      if (retval != 0)
        return false;
      readEndFile();
      return !aborted();
    }

    int PedeApplication::warnings() const
    {
      switch (exitCode) {
        // No warnings
        case 0:
          return 0;
        // Mild warnings (result can be used)
        case 1:
          return 1;
        // Severe warnings (result should not be used - add more data)
        case 2:
          return 2;
        // Extreme warnings (result must not be used - problem not well defined)
        case 3:
          return 3;
        // crashed or aborted:
        default:
          return 101;
      }
    }
  }
}
