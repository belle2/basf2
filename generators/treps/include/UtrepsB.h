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

#include <generators/treps/Particle_array.h>
#include <generators/treps/Sutool.h>
#include <generators/treps/Treps3B.h>

namespace Belle2 {
  /**
   * Input from TREPS generator for ee->eeff
   */

  class UtrepsB : public TrepsB {

  public:

    UtrepsB(void);
    ~UtrepsB() {};

    /** initialization of Pparametrization of pi+pi- partial waves */
    void initg() ;
    Interps b00, b20, b22; // Parameter sets

    virtual double tpform(double, double) const override ; // FormFactor effict
    virtual double tpangd(double, double) override ; // angular distribution for final 2-body case
    virtual int tpuser(TLorentzVector, TLorentzVector, Part_gen*, int); // user decision routine for extra generation conditions.

  private:
    double d2func(double) const;
    double qfunc(double, double) const;

  };

}
