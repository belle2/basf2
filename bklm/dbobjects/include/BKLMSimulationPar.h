/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMSIMULATIONPAR_DB_H
#define BKLMSIMULATIONPAR_DB_H

#include <TObject.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>

namespace Belle2 {

  //! Provides BKLM simulation parameters (from Gearbox)
  class BKLMSimulationPar: public TObject {

    enum {
      c_NDIV     = 5,
      c_MAX_NHIT = 10
    };

  public:

    //! default constructor
    BKLMSimulationPar() {};

    //! constructor
    explicit BKLMSimulationPar(const GearDir&);

    //! default destructor
    ~BKLMSimulationPar();

    //! Static method to get a reference to the singleton SimulationPar instance
    //static BKLMSimulationPar* instance();

    //! Returen BKLM geometry version
    int getVersion() const {return m_version; }

    //! set BKLM simulation parameters version
    void setVersion(int version) { m_version = version; }

    //! Get simulation parameters from Gearbox
    void read(const GearDir&);

    //! Get the maximum global time for a recorded sim hit
    double getHitTimeMax(void) const { return m_HitTimeMax; }

    //! Get number of divisions for phi strips
    int getNPhiDivision(void)  const {return m_NPhiDiv;}

    //! Get number of divisions for z strips
    int getNZDivision(void)  const {return m_NZDiv;}

    //! Get the maximum multiplicity in the RPC strip cumulative prob density functions
    int getMaxMultiplicity(void) const { return c_MAX_NHIT; }

    //!Get number of kind of mutiplicity for phistrips
    int getNPhiMultiplicity(int division) const {return m_NPhiMultiplicity[division];}

    //!Get number of kind of mutiplicity for zstrips
    int getNZMultiplicity(int division) const {return m_NZMultiplicity[division];}

    //!Get weight table for phi
    double getPhiWeight(int division, int nmutiplicity) const { return m_PhiWeight[division][nmutiplicity]; }

    //!Get weight table for z
    double getZWeight(int division, int nmutiplicity) const { return m_ZWeight[division][nmutiplicity]; }

    //! Get the RPC phi-strip cumulative prob density function
    double getPhiMultiplicityCDF(double stripDiv, int mult) const;

    //! Get the RPC z-strip cumulative prob density function
    double getZMultiplicityCDF(double stripDiv, int mult) const;

    //! Get comment
    std::string getBKLMSimulationParComment() const {return m_comment; }

    //! Set comment
    void setBKLMSimulationParComment(const std::string& comment) {m_comment = comment;}

  private:

    //! simulation parameters version
    int m_version;

    //! variable for the maximum global time for a recorded sim hit
    double m_HitTimeMax;

    //! flag for whether the neutron background study is done
    double m_DoBackgroundStudy;

    //! RPC phiStrip multiplicity cumulative probability distribution table
    double m_PhiMultiplicityCDF[c_NDIV + 1][c_MAX_NHIT];

    //! RPC zStrip multiplicity cumulative probability distribution table
    double m_ZMultiplicityCDF[c_NDIV + 1][c_MAX_NHIT];

    //! the number of division for phistrips
    int m_NPhiDiv;

    //! the number of division for zstrips
    int m_NZDiv;

    //! the number of kind of mutiplicity of phistrips in each divison
    int m_NPhiMultiplicity[c_NDIV + 1];

    //! the number of weigths of zstrips in each divison
    int m_NZMultiplicity[c_NDIV + 1];

    //! RPC phiStrip weight table
    double m_PhiWeight[c_NDIV + 1][c_MAX_NHIT];

    //! RPC zStrip weight table
    double m_ZWeight[c_NDIV + 1][c_MAX_NHIT];

    //! static pointer to the singleton instance of this class
    //static BKLMSimulationPar* m_Instance;

    //! flag to indicate if parameters have been loaded yet
    bool m_IsValid;

    //! optional comment
    std::string m_comment;

    ClassDef(BKLMSimulationPar, 2);  /**< ClassDef, must be the last term before the closing {}*/

  };

} // end of namespace Belle2

#endif
