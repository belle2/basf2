/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>
#include <mva/interface/Weightfile.h>
#include <mva/interface/Expert.h>

#include <framework/database/DBObjPtr.h>

#include <vector>
#include <string>
#include <memory>

namespace Belle2 {

  /**
   * This module can be used as a prototype for your own module which uses MVA weightfiles.
   * It is NOT a module which is intended to be used directly
   */
  class MVAPrototypeModule : public Module {
  public:

    /**
     * Constructor
     */
    MVAPrototypeModule();

    /**
     * Initialize the module.
     */
    virtual void initialize() override;

    /**
     * Called at the beginning of a new run
     */
    virtual void beginRun() override;

    /**
     * Called for each event.
     */
    virtual void event() override;

  private:
    /**
     * Initialize mva expert, dataset and features
     * Called every time the weightfile in the database changes in begin run
     */
    void init_mva(MVA::Weightfile& weightfile);

  private:

    std::string m_identifier; /**< database identifier or filename of the weightfile */
    double m_signal_fraction_override; /**< Signal Fraction which should be used. < 0 means use signal fraction of training sample */

    std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>
                                                               m_weightfile_representation; /**< Database pointer to the Database representation of the weightfile */
    std::unique_ptr<MVA::Expert> m_expert; /**< Pointer to the current MVA Expert */
    std::unique_ptr<MVA::SingleDataset> m_dataset; /**< Pointer to the current dataset */
  };

} // Belle2 namespace

