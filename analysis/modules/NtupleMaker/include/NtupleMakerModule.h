/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEDUMPERMODULE_H
#define NTUPLEDUMPERMODULE_H
#include <framework/core/Module.h>
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <analysis/dataobjects/Particle.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <string>
#include <TTree.h>
#include <TFile.h>

namespace Belle2 {

  /** This module writes the properties of given Particle objects
  to a TTree. A ParticleList of reconstructed particles (e.g. B mesons)
  needs to be provided. The properties of any daughter of the provided
  particles can be written to the TTree. The daughters are
  selected with a string parameter, e.g. 'B -> D ^pi (D -> ^K ^pi)'. This
  string will be converted to a DecayDescriptor object by a separate
  parser object (not implemented yet). The properties of the selected
  daughters (momentum, mass, etc.) are evaluated by NtupleTool objects. */
  class NtupleMakerModule : public Module {
  private:
    /** Name of ROOT file for output. */
    std::string m_fileName;
    /** ROOT file for output. */
    static TFile* m_file;
    /** Number of TTrees which are not yet written to TFile. */
    static int m_nTrees;
    /** Name of the TTree. */
    std::string m_treeName;
    /** Comment about the content of the TTree */
    std::string m_comment;
    /** The ROOT TTree for output. */
    TTree* m_tree;
    /** Parameter provided by basf2 script: ntuple tool, decay string, ntuple tool, decay string.... */
    std::vector<std::string> m_toolNames;
    /** Output Tools */
    boost::ptr_vector<NtupleFlatTool> m_tools;
    /** List of the DecayDescriptor objects corresponding to the m_tools. */
    std::vector<DecayDescriptor> m_decaydescriptors;
    /** Name of particle list with reconstructed particles. */
    std::string m_listName;
  public:
    /** Constructor. */
    NtupleMakerModule();
    /** Destructor. */
    ~NtupleMakerModule() {}
    /** Initialises the module. At the moment this contains the
    construction of a DecayDescriptor object. This will by replaced
    by a simple call to the parser. */
    void initialize();
    /** Method called for each event. */
    void event();
    /** Write TTree to file, and close file if necessary. */
    void terminate();
  };
} // end namespace Belle2

#endif // NTUPLEDUMPERMODULE_H
