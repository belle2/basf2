#pragma once

#include <string>
#include <vector>

#include <TObject.h>

using namespace std;
namespace Belle2 {
  /**
   * Class representing Millepede steering.
   */
  class PedeSteering : public TObject {
  public:
    //! Default constructor
    PedeSteering() : commands(), files(), name("PedeSteering.txt") {}
    //! Constructor to directly import command from file
    //! @param filename text file with Pede commands
    explicit PedeSteering(string filename) : commands(), files(), name(filename) {}
    //! Destructor
    virtual ~PedeSteering() {}
    //! Load commands from existing text file
    //! @param filename Path to txt file to be loaded
    void import(string filename);
    //! Add command to the steering
    //! @param line Line with command to be added
    void command(string line);
    //! Compose and write out steering file
    //! @param filename Output steering file name
    string make(string filename = "");
    //! Fix parameter values and set presigmas
    //! @param labels Vector of Pede labels (see GlobalLabel.h)
    //! @param values Vector (of size labels) with values to be set to parameters in labels
    //! @param presigmas Vector (of size labels) with presigmas for parameters (<0 means fixed parameter)
    void fixParameters(vector<int> labels, vector<double> values = vector<double>(), vector<double> presigmas = vector<double>());

    //void addConstraint(double constraint, vector<int> labels, vector<double> coefficients);

    //! Returns a reference to list of binary files to be loaded by Pede
    vector<string>& getFiles() { return files; }
  private:
    //! list command lines
    vector<string> commands;
    //! list of binary files
    vector<string> files;
    //! Name of steering (used as default filename)
    string name;

    ClassDef(PedeSteering, 1) /**< Class representing Millepede steering */

  };

}