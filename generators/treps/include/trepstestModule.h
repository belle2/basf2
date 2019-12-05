/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kiyoshi Hayasaka                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>


namespace Belle2 {
  /**
   * interface to TREPS generator
   *
   * interface to TREPS generator

   */
  class trepstestModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    trepstestModule();

    /** initialize TREPStest */
    virtual void initialize() override;

    /** Produce event */
    virtual void event() override;

    /** Terminate */
    virtual void terminate() override;


  private:

  };
}

