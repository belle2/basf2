/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kiyoshi Hayasaka, Yo Sato                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>
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

    // FormFactor effect */
    double tpform(double, double) const override ;

    // angular distribution for final 2-body case */
    double tpangd(double, double) override ;

    // user decision routine for extra generation conditions. */
    int tpuser(TLorentzVector, TLorentzVector, Part_gen*, int) override ;

  private:
    double d2func(double) const;
    double qfunc(double, double) const;

  };

}
