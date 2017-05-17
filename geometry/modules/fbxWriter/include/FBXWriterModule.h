/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FBXWRITERMODULE_H
#define FBXWRITERMODULE_H

#include <framework/core/Module.h>

#include <string>
#include <fstream>

#include "G4Transform3D.hh"  // Using 'class G4Transform3D' conflicts with typedef here
class G4VPhysicalVolume;
class G4LogicalVolume;
class G4VSolid;
class G4AffineTransform;
class G4Polyhedron;
class HepPolyhedron;

namespace Belle2 {

  /** The FBX-writer module.
   *
   * This module goes through all volumes of the Belle II detector
   * geometry and write an Autodesk FBX file.
   *
   * This module requires a valid geometry. Therefore, a geometry-building
   * module should have been executed before this module is called.
   */
  class FBXWriterModule: public Module {

  public:

    /** Constructor of the module. */
    FBXWriterModule();

    //! Initialize at the start of a job
    void initialize();

    //! Called for each event: this runs the FBX writer only for the first event
    void event();

  private:

    //! Create unique and legal name for each solid, logical volume, physical volume
    void assignName(std::vector<std::string>*, unsigned int, const G4String&, int);

    //! Write FBX definition for each solid's polyhedron
    void writeGeometryNode(G4VSolid*, const std::string&, unsigned long long);

    //! Write FBX definition for each logical volume's color information
    void writeMaterialNode(int, const std::string&);

    //! Write FBX definition for each logical volume
    void writeLVModelNode(G4LogicalVolume*, const std::string&, unsigned long long);

    //! Write FBX definition for each physical volume
    void writePVModelNode(G4VPhysicalVolume*, const std::string&, unsigned long long);

    //! Count the physical volumes, logical volumes, materials and solids (recursive)
    void countEntities(G4VPhysicalVolume*);

    //! Process one physical volume for FBX-node writing (recursive)
    void addModels(G4VPhysicalVolume*, int);

    //! Write FBX connections among all of the nodes in the tree (recursive)
    void addConnections(G4VPhysicalVolume*, int);

    //! Write FBX at the start of the file
    void writePreamble(int, int, int);

    //! Write FBX definition for the solid's polyhedron
    void writePolyhedron(G4VSolid*, G4Polyhedron*, const std::string&, unsigned long long);

    //! Write FBX connection for each logical volume's solid and color info
    void writeSolidToLV(const std::string&, const std::string&, bool, unsigned long long, unsigned long long, unsigned long long);

    //! Write FBX connection for each physical volume's solid and color info (bypass singleton logical volume)
    void writeSolidToPV(const std::string&, const std::string&, bool, unsigned long long, unsigned long long, unsigned long long);

    //! Write FBX connection for the (unique) logical volume of a physical volume
    void writeLVToPV(const std::string&, const std::string&, unsigned long long, unsigned long long);

    //! Write FBX connection for each physical-volume daughter of a parent logical volume
    void writePVToParentLV(const std::string&, const std::string&, unsigned long long, unsigned long long);

    //! Write FBX connection for each physical-volume daughter of a parent physical volume (bypass singleton logical volume)
    void writePVToParentPV(const std::string&, const std::string&, unsigned long long, unsigned long long);

    //! Create polyhedron for a boolean solid (recursive)
    HepPolyhedron* getBooleanSolidPolyhedron(G4VSolid*);

    //! Once-only flag to write FBX only on the first event
    bool m_First;

    //! User-specified flag to select whether to write and re-use logical- and physical-volume
    //! prototypes once (true) or to write duplicates of each such volume (false).
    bool m_UsePrototypes;

    //! User-specified output filename
    std::string m_Filename;

    //! Output file
    std::ofstream m_File;

    //! Modified (legal-character and unique) physical-volume name
    std::vector<std::string>* m_PVName;

    //! Modified (legal-character and unique) logical-volume name
    std::vector<std::string>* m_LVName;

    //! Modified (legal-character and unique) solid name
    std::vector<std::string>* m_SolidName;

    //! Unique identifiers for physical volumes (Model nodes with transformation information)
    std::vector<unsigned long long>* m_PVID;

    //! Unique identifiers for logical volumes (Model nodes with links to Geometry and Material)
    std::vector<unsigned long long>* m_LVID;

    //! Unique identifiers for logical volumes' color information (Material nodes)
    std::vector<unsigned long long>* m_MatID;

    //! Unique identifiers for solids (Geometry nodes)
    std::vector<unsigned long long>* m_SolidID;

    //! Flag to indicate that the logical volume is visible
    std::vector<bool>* m_Visible;

    //! Count of number of instances of each physical volume
    std::vector<unsigned int>* m_PVCount;

    //! Count of number of instances of each logical volume
    std::vector<unsigned int>* m_LVCount;

    //! Count of number of instances of each solid (typically 1)
    std::vector<unsigned int>* m_SolidCount;

    //! Count of number of replicas of each replicated physical volume
    std::vector<unsigned int>* m_PVReplicas;

    //! Count of number of replicas of each logical volume associated with a replicated physical volume
    std::vector<unsigned int>* m_LVReplicas;

    //! Count of number of replicas of each solid (extras for replicas with modified solids)
    std::vector<unsigned int>* m_SolidReplicas;

    //! Flag to indicate that a logical volume is referenced at most once (eligible for bypass)
    std::vector<bool>* m_LVUnique;

  };

} //Belle2 namespace
#endif
