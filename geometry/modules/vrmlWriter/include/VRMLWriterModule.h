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
#include <fstream>

class G4VPhysicalVolume;
class G4LogicalVolume;
class G4VSolid;
class G4Polyhedron;
class G4String;
class HepPolyhedron;

namespace Belle2 {

  /** The VRML-writer module.
   *
   * This module goes through all volumes of the Belle II detector
   * geometry and write a VRML 2.0 file.  Unlike the VRML writer in
   * GEANT4, which creates a flat model, this writes the hierarchical model.
   *
   * This module requires a valid geometry. Therefore,
   * a geometry building module should have been executed before this module is called.
   */
  class VRMLWriterModule: public Module {

  public:

    /** Constructor of the module. */
    VRMLWriterModule();

    //! Initialize at the start of a job
    void initialize() override;

    //! Called for each event: this runs the VRML writer only for the first event
    void event() override;

  private:

    //! Create unique and legal name for each solid
    void assignName(std::vector<std::string>*, unsigned int, const G4String&, int);

    //! Emit VRML for each solid
    void describeSolid(G4VSolid*, const std::string&, bool);

    //! Emit VRML for each logical volume
    void describeLogicalVolume(G4LogicalVolume*, const std::string&, const std::string&, bool);

    //! Access next physical volume in the tree (recursive)
    void describePhysicalVolume(G4VPhysicalVolume*);

    //! Emit VRML for a physical volume (recursive)
    void descendAndDescribe(G4VPhysicalVolume*, const std::string&, int);

    //! Emit VRML for the start of the file
    void writePreamble(void);

    //! Emit VRML for the solid's polyhedron
    void writePolyhedron(const G4Polyhedron*, const std::string&);

    //! Emit VRML for each daughter of a logical volume
    void writePhysicalVolume(const G4VPhysicalVolume*, const std::string&, const std::string&, bool);

    //! Create polyhedron for a boolean solid (recursive)
    HepPolyhedron* getBooleanSolidPolyhedron(G4VSolid*);

    //! Once-only flag to write VRML only on the first event
    bool m_First{true};

    //! User-specified output filename
    std::string m_Filename{"belle2.wrl"};

    //! Output file
    std::ofstream m_File;

    //! Modified (legal-character and unique) physical-volume name
    std::vector<std::string>* m_PVName{nullptr};

    //! Modified (legal-character and unique) logical-volume name
    std::vector<std::string>* m_LVName{nullptr};

    //! Modified (legal-character and unique) solid name
    std::vector<std::string>* m_SolidName{nullptr};

    //! Flag to indicate that a solid can be rendered as a VMRL cylinder
    std::vector<bool>* m_IsCylinder{nullptr};

    //! Indices (in G4PhysicalVolumeStore) of the logical volume's physical-volume daughters
    std::vector<std::vector<int> >* m_PVIndex{nullptr};

    //! Flag to indicate that the logical volume has already been written
    std::vector<bool>* m_LVWritten{nullptr};

    //! Flag to indicate that the physical volume has already been written
    std::vector<bool>* m_PVWritten{nullptr};

  };

} //Belle2 namespace
