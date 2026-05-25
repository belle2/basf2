/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <vector>

namespace Belle2 {


  /**
   * Class for collecting variables related to tau-taubar MC decay process.
   *
   */

  class TauPairDecay : public RelationsObject {

  public:

    /**
     * Default constructor.
     * All private members are set to 0.
     */
    TauPairDecay() : m_pmode(0.0), m_mmode(0.0), m_pprong(0.0), m_mprong(0.0), m_megstar(0), m_pegstar(0),
      m_masses2BMinus(21, 0.0), m_masses3BMinus(35, 0.0), m_masses4BMinus(35, 0.0),
      m_masses2BPlus(21, 0.0), m_masses3BPlus(35, 0.0), m_masses4BPlus(35, 0.0) {};

    // setters
    /**
     * Add ID of positive tau decay
     *
     * @param pmode ID of generated decay.
     */
    void addTauPlusIdMode(int pmode);

    /**
     * Add ID of negative tau decay
     *
     * @param mmode ID of generated decay.
     */
    void addTauMinusIdMode(int mmode);

    /**
     * Add prong of positive tau decay
     *
     * @param pprong prong of generated tau decay.
     */
    void addTauPlusMcProng(int pprong);

    /**
     * Add prong of negative tau decay
     *
     * @param mprong prong of generated tau decay.
     */
    void addTauMinusMcProng(int mprong);

    /**
     * Add energy of radiated photon from negative tau decay [in tau- rest frame]
     *
     * @param megstar egstar of generated tau- decay.
     */
    void addTauMinusEgstar(double megstar);

    /**
     * Add energy of radiated photon from positive tau decay [in tau+ rest frame]
     *
     * @param pegstar egstar of generated tau+ decay.
     */
    void addTauPlusEgstar(double pegstar);

    /**
     * Get ID of positive tau decay
     *
     * @return Id of generated decay
     */
    int getTauPlusIdMode(void) const
    {
      return m_pmode;
    }

    /**
     * Get ID of negative tau decay
     *
     * @return Id of generated decay
     */
    int getTauMinusIdMode(void) const
    {
      return m_mmode;
    }

    /**
     * Get prong of positive tau decay
     *
     * @return Prong of generated tau decay
     */
    int getTauPlusMcProng(void) const
    {
      return m_pprong;
    }

    /**
     * Get prong of negative tau decay
     *
     * @return Prong of generated tau decay
     */
    int getTauMinusMcProng(void) const
    {
      return m_mprong;
    }

    /**
     * Get energy of radiated photon from negative tau decay [in tau- rest frame]
     *
     * @return Energy of radiated photon from generated tau- decay
     */
    double getTauMinusEgstar(void) const
    {
      return m_megstar;
    }

    /**
     * Get energy of radiated photon from positive tau decay [in tau+ rest frame]
     *
     * @return Energy of radiated photon from generated tau+ decay
     */
    double getTauPlusEgstar(void) const
    {
      return m_pegstar;
    }

    /** Add 2-body invariant masses for tau- decay */
    void addTauMinusMasses2Body(const std::vector<double>& masses);

    /** Add 3-body invariant masses for tau- decay */
    void addTauMinusMasses3Body(const std::vector<double>& masses);

    /** Add 4-body invariant masses for tau- decay */
    void addTauMinusMasses4Body(const std::vector<double>& masses);

    /** Add 2-body invariant masses for tau+ decay */
    void addTauPlusMasses2Body(const std::vector<double>& masses);

    /** Add 3-body invariant masses for tau+ decay */
    void addTauPlusMasses3Body(const std::vector<double>& masses);

    /** Add 4-body invariant masses for tau+ decay */
    void addTauPlusMasses4Body(const std::vector<double>& masses);

    /** Get 2-body invariant masses for tau- decay */
    const std::vector<double>& getTauMinusMasses2Body(void) const
    {
      return m_masses2BMinus;
    }

    /** Get 3-body invariant masses for tau- decay */
    const std::vector<double>& getTauMinusMasses3Body(void) const
    {
      return m_masses3BMinus;
    }

    /** Get 4-body invariant masses for tau- decay */
    const std::vector<double>& getTauMinusMasses4Body(void) const
    {
      return m_masses4BMinus;
    }

    /** Get 2-body invariant masses for tau+ decay */
    const std::vector<double>& getTauPlusMasses2Body(void) const
    {
      return m_masses2BPlus;
    }

    /** Get 3-body invariant masses for tau+ decay */
    const std::vector<double>& getTauPlusMasses3Body(void) const
    {
      return m_masses3BPlus;
    }

    /** Get 4-body invariant masses for tau+ decay */
    const std::vector<double>& getTauPlusMasses4Body(void) const
    {
      return m_masses4BPlus;
    }

  private:

    // persistent data members
    int m_pmode; /**< Decay ID of positive tau lepton decay */
    int m_mmode; /**< Decay ID of negative tau lepton decay */
    int m_pprong; /**< Prong of positive tau lepton decay */
    int m_mprong; /**< Prong of negative tau lepton decay */
    double m_megstar; /**< Energy of photon from negative tau decay*/
    double m_pegstar; /**< Energy of photon from positive tau decay*/
    std::vector<double> m_masses2BMinus; /**< tau- 2-body invariant masses (10 fixed slots) */
    std::vector<double> m_masses3BMinus; /**< tau- 3-body invariant masses (10 fixed slots) */
    std::vector<double> m_masses4BMinus; /**< tau- 4-body invariant masses (5 fixed slots) */
    std::vector<double> m_masses2BPlus; /**< tau+ 2-body invariant masses (10 fixed slots) */
    std::vector<double> m_masses3BPlus; /**< tau+ 3-body invariant masses (10 fixed slots) */
    std::vector<double> m_masses4BPlus; /**< tau+ 4-body invariant masses (5 fixed slots) */

    ClassDef(TauPairDecay, 3) /**< class definition */

  };


} // end namespace Belle2

