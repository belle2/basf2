// -*- C++ -*-
//
// Package:     <package>
// Module:      Particle_array
//
// Description: <one line class summary>
//   Include Class Part_cont  including the properties of the particle
// Implimentation:
//     <Notes on implimentation>
//
// Author:      Sadaharu Uehara
// Created:     Thu May  8 14:42:33 JST 1997
// $Id$
//
// Revision history
//
// $Log$


// user include files
#include <generators/treps/Particle_array.h>

namespace Belle2 {

  Part_cont::Part_cont(int icodex, double pmassx, double pchargx,
                       int ndecx, double pwidthx)
  {
    icode = icodex ;
    pmass = pmassx ;
    pcharg = pchargx ;
    ndec = ndecx ;
    pwidth = pwidthx ;
  }

  Part_cont::Part_cont(int icodex, double pmassx, double pchargx)
  {
    icode = icodex ;
    pmass = pmassx ;
    pcharg = pchargx ;
  }

} // namespace Belle
