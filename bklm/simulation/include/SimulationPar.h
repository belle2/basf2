/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SIMULATIONPAR_H
#define SIMULATIONPAR_H

namespace Belle2 {

  namespace bklm {

    //! Provides BKLM simulation parameters (from Gearbox)
    class SimulationPar {

      enum {
        c_NDIV     = 5,
        c_MAX_NHIT = 10
      };

    public:

      //! Static method to get a reference to the singleton SimulationPar instance
      static SimulationPar* instance();

      //! Get simulation parameters from Gearbox
      void read();

      //! Get the flag for whether parameters have been loaded by BKLMParamLoader
      bool isValid(void) { if (!m_IsValid) read(); return m_IsValid; }

      //! Get the maximum global time for a recorded sim hit
      double getHitTimeMax(void) const { return m_HitTimeMax; }

      //! Get the maximum multiplicity in the RPC strip cumulative prob density functions
      int getMaxMultiplicity(void) const { return c_MAX_NHIT; }

      //! Get the RPC phi-strip cumulative prob density function
      double getPhiMultiplicityCDF(double stripDiv, int mult) const;

      //! Get the RPC z-strip cumulative prob density function
      double getZMultiplicityCDF(double stripDiv, int mult) const;

    private:

      //! Hidden constructor
      SimulationPar();

      //! Hidden copy constructor
      SimulationPar(SimulationPar&);

      //! Hidden copy assignment
      SimulationPar& operator=(const SimulationPar&);

      //! Hidden destructor
      ~SimulationPar();

      //! variable for the maximum global time for a recorded sim hit
      double m_HitTimeMax;

      //! RPC phiStrip multiplicity cumulative probability distribution table
      double m_PhiMultiplicityCDF[c_NDIV + 1][c_MAX_NHIT];

      //! RPC zStrip multiplicity cumulative probability distribution table
      double m_ZMultiplicityCDF[c_NDIV + 1][c_MAX_NHIT];

      //! static pointer to the singleton instance of this class
      static SimulationPar* m_Instance;

      //! flag to indicate if parameters have been loaded yet
      bool m_IsValid;

    };

  } // end of namespace bklm

} // end of namespace Belle2

#endif // SIMULATIONPAR_H
