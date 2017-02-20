/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jan Strube, Sam Cunliffe                                 *
 * (jan.strube@pnnl.gov samuel.cunliffe@pnnl.gov)                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <top/dbobjects/TOPCalChannelMask.h> // Umberto's database object


namespace Belle2 {
  /**
   * Masks dead PMs from the reconstruction
   *
   *    *
   */
  class TOPDeadChannelMaskerModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    TOPDeadChannelMaskerModule();

    /**
     * Destructor
     */
    virtual ~TOPDeadChannelMaskerModule();

    /**
     * FIXME add doc
     */
    virtual void initialize();

    /**
     * FIXME add doc
     */
    virtual void beginRun();

    /**
     * FIXME add doc
     */
    virtual void event();

    /**
     * FIXME add doc
     */
    virtual void endRun();

    /**
     * FIXME add doc
     */
    virtual void terminate();


  private:

  };
}
