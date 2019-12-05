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
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/MCParticleGraph.h>
#include <generators/utilities/InitialParticleGeneration.h>
#include <string>

#include<TVector3.h>
#include<TLorentzVector.h>

// user include files
#include <generators/treps/Particle_array.h>
#include <generators/treps/Sutool.h>
#include <generators/treps/Treps3B.h>

namespace Belle2 {
  /**
   * Input from TREPS generator for ee->eeff
   */

  class UtrepsB : public TrepsB {

  public:

    /*** Constructor: Sets the description, the properties and the parameters of the module.
     */
    UtrepsB(void);
    ~UtrepsB() { };

    /** initialization for trepsinput */
    virtual void initialize() { };

    /** initialization for trepsinput */
    virtual void terminate() { };

    /** input event from TREPS */
    virtual void event() { };

    virtual double tpform(double, double) const override ;
    virtual double tpangd(double, double) override ;
    virtual int tpuser(TLorentzVector, TLorentzVector,
                       Part_gen*, int);

    Interps b00, b20, b22;

    void initg() ;

  private:
    double d2func(double) const;
    double qfunc(double, double) const;

  };

}
