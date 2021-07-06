/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
