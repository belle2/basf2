/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMSIMULATIONPAR_H
#define BKLMSIMULATIONPAR_H

#include <framework/gearbox/GearDir.h>

namespace Belle2 {

  //! Provides BKLM simulation parameters (from Gearbox)
  class BKLMSimulationPar {

    enum {
      c_NDIV     = 5,
      c_MAX_NHIT = 10
    };

  public:

    //! Static method to get a reference to the singleton BKLMSimulationPar instance
    static BKLMSimulationPar* instance();

    //! Get simulation parameters from Gearbox
    void read(const GearDir&, unsigned int, bool);

    //! Get the flag for whether parameters have been loaded by BKLMParamLoader
    bool isValid(void) const { return m_IsValid; }

    //! Get the maximum global time for a recorded sim hit
    double getHitTimeMax(void) const { return m_HitTimeMax; }

    //! Get the random number seed for the RPC strip-multiplicity algorithm
    unsigned int getRandomSeed(void) const { return m_RandomSeed; }

    //! Get the flag that controls whether to do the neutron-background study
    double getDoBackgroundStudy(void) const { return m_DoBackgroundStudy; }

    //! Get the maximum multiplicity in the RPC strip cumulative prob density functions
    int getMaxMultiplicity(void) const { return c_MAX_NHIT; }

    //! Get the RPC phi-strip cumulative prob density function
    double getPhiMultiplicityCDF(double stripDiv, int mult) const;

    //! Get the RPC z-strip cumulative prob density function
    double getZMultiplicityCDF(double stripDiv, int mult) const;

  private:

    //! Hidden constructor
    BKLMSimulationPar();

    //! Hidden copy constructor
    BKLMSimulationPar(BKLMSimulationPar&);

    //! Hidden copy assignment
    BKLMSimulationPar& operator=(const BKLMSimulationPar&);

    //! Hidden destructor
    ~BKLMSimulationPar();

    //! variable for the maximum global time for a recorded sim hit
    double m_HitTimeMax;

    //! variable for the random number seed used in RPC strip multiplicity algorithm
    unsigned int m_RandomSeed;

    //! flag for whether the neutron background study is done
    double m_DoBackgroundStudy;

    //! RPC phiStrip multiplicity cumulative probability distribution table
    double m_PhiMultiplicityCDF[c_NDIV+1][c_MAX_NHIT];

    //! RPC zStrip multiplicity cumulative probability distribution table
    double m_ZMultiplicityCDF[c_NDIV+1][c_MAX_NHIT];

    //! static pointer to the singleton instance of this class
    static BKLMSimulationPar* m_Instance;

    //! flag to indicate if parameters have been loaded yet
    bool m_IsValid;

  };

} // end of namespace Belle2

#endif // BKLMSIMULATIONPAR_H
