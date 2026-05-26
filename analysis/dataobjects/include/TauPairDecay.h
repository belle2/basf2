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
    TauPairDecay() : m_pmode(0.0), m_mmode(0.0), m_pprong(0.0), m_mprong(0.0), m_megstar(0), m_pegstar(0) {};

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

    /**
     * Add list of daughter particle indices from negative tau decay
     *
     * @param dau_tauminus vector of daughter particle indices from tau-.
     */
    void addTauMinusDaughters(const std::vector<int>& dau_tauminus);

    /**
     * Add list of daughter particle indices from positive tau decay
     *
     * @param dau_tauplus vector of daughter particle indices from tau+.
     */
    void addTauPlusDaughters(const std::vector<int>& dau_tauplus);

    /**
     * Get list of daughter particle indices from negative tau decay
     *
     * @return Vector of indices of daughter particles from tau-.
     */
    std::vector<int> getTauMinusDaughters(void) const
    {
      return m_vec_dau_tauminus;
    }

    /**
     * Get list of daughter particle indices from positive tau decay
     *
     * @return Vector of indices of daughter particles from tau+.
     */
    std::vector<int> getTauPlusDaughters(void) const
    {
      return m_vec_dau_tauplus;
    }

  private:

    // persistent data members
    int m_pmode; /**< Decay ID of positive tau lepton decay */
    int m_mmode; /**< Decay ID of negative tau lepton decay */
    int m_pprong; /**< Prong of positive tau lepton decay */
    int m_mprong; /**< Prong of negative tau lepton decay */
    double m_megstar; /**< Energy of photon from negative tau decay*/
    double m_pegstar; /**< Energy of photon from positive tau decay*/
    std::vector<int> m_vec_dau_tauminus; /**< Indices of daughter particles from tau- decay */
    std::vector<int> m_vec_dau_tauplus; /**< Indices of daughter particles from tau+ decay */

    ClassDef(TauPairDecay, 3) /**< class definition */

  };


} // end namespace Belle2

