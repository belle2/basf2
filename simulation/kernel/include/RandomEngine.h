/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#ifndef SIMULATION_KERNEL_RANDOMENGINE_H
#define SIMULATION_KERNEL_RANDOMENGINE_H

#include <CLHEP/Random/Random.h>
#include <framework/core/RandomGenerator.h>

namespace Belle2 {
  /** Interface class to make Geant4 use the Belle2 RandomGenerator */
  class RandomEngine final: public CLHEP::HepRandomEngine {
  public:
    /** Constructor expecting a reference to the Belle2::RandomGenerator */
    RandomEngine(): CLHEP::HepRandomEngine() {}
    /** return a flat distributed double in (0,1), both excluded */
    double flat() override final { return dynamic_cast<RandomGenerator&>(*gRandom).random01(); }
    /** fill an array of doubles with values in (0,1), both exluded
     * @param size number of doubles to generate
     * @param vect pointer to buffer at least sizeof(double)*size bytes large
     */
    void flatArray(const int size, double* vect) override final { dynamic_cast<RandomGenerator&>(*gRandom).RndmArray(size, vect); }
    /** set the seed, ignored */
    void setSeed(long, int) override final {}
    /** set array of seed values, ignored */
    void setSeeds(const long*, int) override final {}
    /** save status to file, ignored */
    void saveStatus(const char[]) const override final {}
    /** restore status from file, ignored */
    void restoreStatus(const char[]) override final {}
    /** show status, ignored */
    void showStatus() const override final {}
    /** return name of the generator */
    std::string name() const override final { return "Belle2::RandomGenerator"; }
    /** put call, whatever that does. If we don't override it we get useless messages */
    std::vector<unsigned long> put() const override final { return std::vector<unsigned long>(); }
    /** put call, whatever that does. If we don't override it we get useless messages */
    std::ostream& put(std::ostream& o) const override final { return o; }
    /** get call, I assume same as above */
    bool get(const std::vector<unsigned long>&) override final { return false; }
    /** get call, I assume same as above */
    std::istream& get(std::istream& i) override final { return i; }
  };

} //Belle2 namespace
#endif // SIMULATION_KERNEL_RANDOMENGINE_H
