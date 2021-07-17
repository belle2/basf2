/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TObject.h>

#include <string>
#include <vector>

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
    explicit PedeSteering(const std::string& filename) : commands(), files(), name(filename) {}
    //! Destructor
    virtual ~PedeSteering() {}
    //! Load commands from existing text file
    //! @param filename Path to txt file to be loaded
    void import(std::string filename);
    //! Add command to the steering
    //! @param line Line with command to be added
    void command(std::string line);
    //! Compose and write out steering file
    //! @param filename Output steering file name
    std::string make(std::string filename = "");
    //! Fix parameter values and set presigmas
    //! @param labels Vector of Pede labels (see GlobalLabel.h)
    //! @param values Vector (of size labels) with values to be set to parameters in labels
    //! @param presigmas Vector (of size labels) with presigmas for parameters (<0 means fixed parameter)
    void fixParameters(std::vector<int> labels, std::vector<double> values = {}, std::vector<double> presigmas = {});

    //void addConstraint(double constraint, vector<int> labels, vector<double> coefficients);

    //! Add a file (optionally with weight) to list of binary files
    void addFile(std::string filename, double weight = 1.);

    //! Clear list of files
    void clearFiles() {files.clear();}

  private:
    //! list command lines
    std::vector<std::string> commands;
    //! list of binary files
    std::vector<std::string> files;
    //! Name of steering (used as default filename)
    std::string name;

    ClassDef(PedeSteering, 1) /**< Class representing Millepede steering */

  };

}
