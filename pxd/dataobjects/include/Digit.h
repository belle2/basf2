/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka, Zbynek Drasal                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef DIGIT_H
#define DIGIT_H

#include <pxd/dataobjects/HitSorter.h>

#include <map>

namespace Belle2 {

  /** Prototype of relation lists : index to mother object's StoreArray + weight.*/
  typedef std::map<StoreIndex, float> StoreRelationMap;
  typedef std::map<StoreIndex, float>::const_iterator StoreRelationMapItr;


  //------------------------------------------------------------------------------
  // ProtoDigit class is used internally for handling of digits and relations
  // in digitization and clustering.
  //----------------------------------------------------------------------------

  /**
   * ProtoDigit structure - for processing of digits in maps.
   *
   * Contains a list of relations to MCParticles, but has no sensor information
   * data, which is expected to be provided by the map key.
   */
  struct Digit {

    /**
     * Default constructor makes an empty digit.
     */
    Digit(): charge(0), sourceHits() {;}

    /**
     * Digits are filled by adding collected charge.
     * We also record the particle (hit) that deposited the charge.
     */
    void add(double aCharge, StoreIndex aHit)
    { charge += aCharge; sourceHits[aHit] += aCharge; }

    /**
     * Prints contents of the digit.
     */
    void print() const;

    double charge;                 /**< Charge deposited in the pixel.*/
    StoreRelationMap sourceHits;   /**< Deposited charge by particle.*/
  };

  /** Digits by pixel ID. */
  typedef std::map<int, Digit> DigitMap;
  typedef std::map<int, Digit>::iterator DigitMapItr;


} // end namespace Belle2

#endif
