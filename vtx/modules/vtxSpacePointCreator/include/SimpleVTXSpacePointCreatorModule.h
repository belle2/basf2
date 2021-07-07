/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
//#ifndef SIMPLEVTXSPACEPOINTCREATORMODULE_H
//#define SIMPLEVTXSPACEPOINTCREATORMODULE_H

#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>
#include <vtx/dataobjects/VTXSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <tracking/spacePointCreation/SpacePoint.h>

#include <string>


namespace Belle2 {
  //namespace VTX {
  /**
   * VTX spacepoints
   *
   * Creates Spacepoints from VTXSimHits
   *
   */
  class SimpleVTXSpacePointCreatorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     * TO BE REWORKED
     */
    SimpleVTXSpacePointCreatorModule();

    /**  */
    virtual ~SimpleVTXSpacePointCreatorModule();

    /**  */
    virtual void initialize() override;

    /**  */
    virtual void beginRun() override;

    /**  */
    virtual void event() override;

    /**  */
    virtual void endRun() override;

    /**  */
    virtual void terminate() override;


  private:
    /** Event number */
    int Event = 0;
  };

  //}
}

//#endif /* SIMPLEVTXSPACEPOINTCREATORMODULE_H */
