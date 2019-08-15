/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sourav Dey, Abi Soffer                                   *
 *Past Contributors: Omer Benami                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#pragma once
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <string>


#include <analysis/DecayDescriptor/DecayDescriptor.h>


namespace Belle2 {
  /**
   * Calculates distance between two vertices, distance of closest approach between a vertex and a track, distance of closest approach between a vertex and btube
   */
  class DistanceCalculatorModule : public Module {

  public:
    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    DistanceCalculatorModule();
    /** Destructor */
    virtual ~DistanceCalculatorModule();

    /** declare data store elements */
    virtual void initialize() override;

    /** process event */
    virtual void event() override;
    //    virtual void terminate() override;

  private:

    std::string m_listName; /**< name of particle list */
    std::string m_decayString;  /**< decay string */
    DecayDescriptor m_decayDescriptor;  /**< decay descriptor which specifies which particles are used to calculate the distance */
    std::string m_mode; /**< option string */
  };
}


