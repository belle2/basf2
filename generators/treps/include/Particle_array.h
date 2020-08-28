// -*- C++ -*-
//
// Package:     <package>
// Module:      Particle_array
//
// Description: <one line class summary>
//
// Usage:
//    Headers for the classes Part_cont and Part_gen
//
// Author:      Sadaharu Uehara
// Created:     Thu May  8 14:51:28 JST 1997
// $Id$
//
// Revision history
//
// $Log$



#pragma once

#include <TLorentzVector.h>

namespace Belle2 {


  class Part_cont {
  public:
    // constants, enums and typedefs
    int icode;
    double pmass, pcharg ;
    int ndec ;
    double pwidth ;
    double pmassp ;

    // Constructors and destructor
    Part_cont(): icode(0), pmass(0.), pcharg(0.),
      ndec(0), pwidth(0.), pmassp(0.) {};
    Part_cont(int, double, double, int, double);
    Part_cont(int, double, double);
    ~Part_cont(void) {};

  };

  class Part_gen {

  public:
    // constants, enums and typedefs
    Part_cont part_prop ;
    TLorentzVector p ;

    // Constructors and destructor
    Part_gen() {};
    ~Part_gen() {};

  };
} // namespace Belle


