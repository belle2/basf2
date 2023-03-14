/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/database/DBObjPtr.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <framework/datastore/StoreArray.h>

// MVA
#include <mva/interface/Expert.h>
#include <mva/interface/Dataset.h>

// ANALYSIS
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>
#include <analysis/dbobjects/ChargedPidMVAWeights.h>

namespace Belle2 {

  /**
   *
   * This module evaluates the response of a multi-class MVA trained for global charged particle identification.
   *
   * It takes the Particle objects in the input charged stable particles' ParticleLists,
   * calculates the MVA per-class score using the appropriate xml weight file, and adds it as ExtraInfo to the Particle objects.
   *
   */
  class ChargedPidMVAMulticlassModule : public Module {

    typedef std::vector<std::unique_ptr<MVA::Expert> > ExpertsList; /**< Typedef */
    typedef std::vector<std::unique_ptr<MVA::SingleDataset> > DatasetsList; /**< Typedef */
    typedef std::vector<std::unique_ptr<Variable::Cut>> CutsList; /**< Typedef */
    typedef std::vector< std::vector<const Variable::Manager::Var*> > VariablesLists; /**< Typedef */

  public:

    /**
     * Constructor, for setting module description and parameters.
     */
    ChargedPidMVAMulticlassModule();

    /**
     * Destructor, use this to clean up anything you created in the constructor.
     */
    virtual ~ChargedPidMVAMulticlassModule();

    /**
     * Use this to initialize resources or memory your module needs.
     *
     * Also register any outputs of your module (StoreArrays, StoreObjPtrs, relations) here,
     * see the respective class documentation for details.
     */
    virtual void initialize() override;

    /**
     * Called once before a new run begins.
     *
     * This method gives you the chance to change run dependent constants like alignment parameters, etc.
     */
    virtual void beginRun() override;

    /**
     * Called once for each event.
     *
     * This is most likely where your module will actually do anything.
     */
    virtual void event() override;

  private:

    /**
     * 1. Check if a payload is found in the database.
     * 2. Check the MVA weights for consistency every time they change in the database.
     * 3. Load MVA weight files, and set MVA::Expert and MVA::SingleDataset objects for each file found.
    */
    void initializeMVA();

    /**
     * Split the particle list name in "particleName", "particleLabel",
     * and return the particle name w/o the charge +/- label.
     * This corresponds to the name as given in the evt.pdl
    */
    const std::string getParticleName(const ParticleList* pList) const
    {

      auto fullName = pList->getParticleListName();
      auto delimiter(":");
      auto signedName = fullName.substr(0, fullName.find(delimiter));
      signedName.pop_back();

      return signedName;
    }

  private:
    /**
     * StoreArray of Particles
     */
    StoreArray<Particle> m_particles;

    /**
     * The input list of DecayStrings, where each selected (^) daughter should correspond to a standard charged ParticleList,
     * e.g. `['Lambda0:sig -> ^p+ ^pi-', 'J/psi:sig -> ^mu+ ^mu-']`. One can also directly pass a list of
     * standard charged ParticleLists, e.g. `['e+:my_electrons', 'pi+:my_pions']`.
     * Note that charge-conjugated ParticleLists will automatically be included.
     */
    std::vector<std::string> m_decayStrings;

    /**
     * The name of the database payload object with the MVA weights.
     */
    std::string m_payload_name;

    /**
     * Flag to specify if we use a charge-independent training.
     */
    bool m_charge_independent;

    /**
     * Flag to specify if we use an ECL-only based training.
     */
    bool m_ecl_only;

    /**
     * The event information. Used for debugging purposes.
     */
    StoreObjPtr<EventMetaData> m_event_metadata;

    /**
     * Interface to get the database payload with the MVA weight files.
     * The payload class has a method to retrieve the correct weightfile representation
     * given a reconstructed particle's (polar angle, p, charge).
     * Note that the theta of the track helix extraplolated at the ECL entry surface
     * is used if the particle doesn't have an ECL cluster match.
     */
    std::unique_ptr<DBObjPtr<ChargedPidMVAWeights>> m_weightfiles_representation;

    /**
     * List of MVA::Expert objects.
     * One Expert to be stored for each xml file found in the database, i.e. for each training category.
     *
     */
    ExpertsList m_experts;

    /**
     * List of MVA::SingleDataset objects.
     * One DS to be stored for each xml file found in the database, i.e. for each training category.
     */
    DatasetsList m_datasets;

    /**
     * List of Cut objects.
     * One Cut to be stored for each training category.
     */
    CutsList m_cuts;

    /**
     * List of lists of feature variables.
     * One list of lists to be stored for each xml file found in the database, i.e. for each training category.
     */
    VariablesLists m_variables;

    /**
     * List of lists of spectator variables.
     * One list of lists to be stored for each xml file found in the database, i.e. for each training category.
     */
    VariablesLists m_spectators;

    /**
     * List of MVA class names.
     */
    std::vector<std::string> m_classes;

    /**
     * Map with standard charged particles' info. For convenience.
     */
    std::map<int, std::string> m_stdChargedInfo = {
      { Const::electron.getPDGCode(), "electron" },
      { Const::muon.getPDGCode(), "muon" },
      { Const::pion.getPDGCode(), "pion" },
      { Const::kaon.getPDGCode(), "kaon" },
      { Const::proton.getPDGCode(), "proton" },
      { Const::deuteron.getPDGCode(), "deuteron" }
    };

    /**
     * Set variable aliases needed by the MVA. Fallback to this if no aliases map found in payload.
     */
    void registerAliasesLegacy();

    /**
     * Set variable aliases needed by the MVA. Read from payload.
     */
    void registerAliases();

  };
}
