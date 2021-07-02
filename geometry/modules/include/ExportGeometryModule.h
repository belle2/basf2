/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <string>


namespace Belle2 {

  /**
   * The ExportGeometry module.
   *
   * Takes the current detector geometry from the memory and
   * saves it to a root file.
  */
  class ExportGeometryModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the description and the parameters of the module.
     */
    ExportGeometryModule();

    /** Destructor. */
    virtual ~ExportGeometryModule();

    /**
     * Checks the validity of the module parameters.
     * Checks if the filepath of the given filename exists.
     */
    virtual void initialize() override;

    /** Saves the geometry to a root file for every run. */
    virtual void beginRun() override;


  protected:


  private:

    std::string m_filenameROOT; /**< The filename of the ROOT file into which the geometry is saved. */
    bool m_exportAsGDML{false}; /**< Export the geometry as GDML format */
    bool m_gdmlAdresses{true}; /**< If true make the xml names unique by prepending the pointer address */

  };
}
