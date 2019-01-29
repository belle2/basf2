/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2019 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Xing-Yu Zhou                                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once
#include <framework/core/Module.h>
#include <string>
#include <TFile.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/pcore/RootMergeable.h>
#include <TTree.h>

namespace Belle2 {

  /** Creates and fills a tree for the topology information of MC generated particles. To be specific, the information involves the following three variables:
  nMCGen (number of MC generated particles in a given event),
  MCGenPDG (array of the PDG codes of MC generated particles in a given event), and
  MCGenMothIndex (array of the mother indexes of MC generated particles in a given event).

  Along with the variables, the event meta data, namely __experiment__, __run__, __event__, are always stored. In case a decayString is provided, __candidate__ and __ncandidates__ will be stored as well.

  MCGenPDG and MCGenMothIndex are not scalar variables but array variables with variable length equal to nMCGen. Hence, they can not be stored to flat ntuples with the module VariablesToNtuple. That's the reason why the module is developed. Note that the module is referred to as MCGenTopoToTree instead of MCGenTopoToNtuple, in order to avoid mistaking the tree as a flat ntuple with only scalar values stored.

  To keep consistent with the case of the module VariablesToNtuple: if a decayString is provided, one entry per candidate will be filled to the tree; otherwise, one entry per event will be filled to the tree. */

  class MCGenTopoToTreeModule : public Module {

  private:

    /** Particle list to be investigated */
    std::string m_particleList;

    /** Name of the tree for output */
    std::string m_treeName;

    /** Name of the file for output */
    std::string m_fileName;

    /** Maximum permissible number of MC generated particles in a given event. This number is used as the array length of the following three temporary variables in the event method: MCGenPDG, MCGenMothIndex and MCGenIndex, as well as the following two member variables in the initialize method: m_MCGenPDG and m_MCGenMothIndex. For details, please refer to the two methods of the module. */
    int m_nMCGenMax;

    /** Pointer to the file for output */
    std::shared_ptr<TFile> m_file{nullptr};

    /** Pointer to the tree for output */
    StoreObjPtr<RootMergeable<TTree>> m_tree;

    /** Experiment number */
    int m_experiment;

    /** Run number */
    int m_run;

    /** Event number */
    int m_event;

    /** Candidate number */
    int m_candidate;

    /** Number of candidates in a given event */
    int m_ncandidates;

    /** Number of MC generated particles in a given event */
    int m_nMCGen;

    /** Pointer to the array of the PDG codes of MC generated particles in a given event */
    int* m_MCGenPDG;

    /** Pointer to the array of the mother indexes of MC generated particles in a given event */
    int* m_MCGenMothIndex;

  public:

    /** Constructor to set the description and property flags, define Parameters and assign initial values to private member variables. */
    MCGenTopoToTreeModule();

    /** Method to create the file, the tree and the branches. */
    virtual void initialize() override;

    /** Method to calculates the values of the branches and fill them to the tree. The method will be called once for each event. */
    virtual void event() override;

    /** Method to write the tree to the file and reset the file. */
    virtual void terminate() override;

  };

} // end namespace Belle2
