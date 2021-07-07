/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Belle 2 headers. */
#include <framework/gearbox/GearDir.h>

/* ROOT headers. */
#include <TObject.h>

namespace Belle2 {

  //! Provides BKLM simulation parameters
  class BKLMSimulationPar: public TObject {

    enum {
      c_NDIV     = 5,
      c_MAX_NHIT = 10
    };

  public:

    //! Default constructor
    BKLMSimulationPar()
    {
    }

    //! Constructor
    explicit BKLMSimulationPar(const GearDir&);

    //! Default destructor
    ~BKLMSimulationPar();

    //! Get simulation parameters from Gearbox
    void read(const GearDir&);

    //! Get the maximum global time for a recorded sim hit
    double getHitTimeMax(void) const
    {
      return m_HitTimeMax;
    }

    //! Get number of divisions for phi strips
    int getNPhiDivision(void)  const
    {
      return m_NPhiDiv;
    }

    //! Get number of divisions for z strips
    int getNZDivision(void)  const
    {
      return m_NZDiv;
    }

    //! Get the maximum multiplicity in the RPC strip cumulative prob density functions
    int getMaxMultiplicity(void) const
    {
      return c_MAX_NHIT;
    }

    //! Get number of kind of mutiplicity for phistrips
    int getNPhiMultiplicity(int division) const
    {
      return m_NPhiMultiplicity[division];
    }

    //! Get number of kind of mutiplicity for zstrips
    int getNZMultiplicity(int division) const
    {
      return m_NZMultiplicity[division];
    }

    //! Get weight table for phi
    double getPhiWeight(int division, int nmutiplicity) const
    {
      return m_PhiWeight[division][nmutiplicity];
    }

    //! Get weight table for z
    double getZWeight(int division, int nmutiplicity) const
    {
      return m_ZWeight[division][nmutiplicity];
    }

    //! Get the RPC phi-strip cumulative prob density function
    double getPhiMultiplicityCDF(double stripDiv, int mult) const;

    //! Get the RPC z-strip cumulative prob density function
    double getZMultiplicityCDF(double stripDiv, int mult) const;

  private:

    //! Maximum global time for a recorded sim hit
    double m_HitTimeMax;

    //! RPC phiStrip multiplicity cumulative probability distribution table
    double m_PhiMultiplicityCDF[c_NDIV + 1][c_MAX_NHIT];

    //! RPC zStrip multiplicity cumulative probability distribution table
    double m_ZMultiplicityCDF[c_NDIV + 1][c_MAX_NHIT];

    //! Number of division for phistrips
    int m_NPhiDiv;

    //! Number of division for zstrips
    int m_NZDiv;

    //! Number of kind of mutiplicity of phistrips in each divison
    int m_NPhiMultiplicity[c_NDIV + 1];

    //! Number of weigths of zstrips in each divison
    int m_NZMultiplicity[c_NDIV + 1];

    //! RPC phiStrip weight table
    double m_PhiWeight[c_NDIV + 1][c_MAX_NHIT];

    //! RPC zStrip weight table
    double m_ZWeight[c_NDIV + 1][c_MAX_NHIT];

    /** Class version. */
    ClassDef(BKLMSimulationPar, 3);

  };

} // end of namespace Belle2
