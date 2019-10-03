/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kiyoshi Hayasaka, Michel Villanueva                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>

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
    TauPairDecay() : m_pmode(0.0), m_mmode(0.0), m_pprong(0.0), m_mprong(0.0) {};

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
    void addTauMinusIdMode(int pmode);

    /**
     * Add prong of positive tau decay
     *
     * @param pprong prong of generated tau decay.
     */
    void addTauPlusMcProng(int pprong);

    /**
     * Add prong of negative tau decay
     *
     * @param mmode prong of generated tau decay.
     */
    void addTauMinusMcProng(int mprong);

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

  private:

    // persistent data members
    int m_pmode; /**< Decay ID of positive tau lepton decay */
    int m_mmode; /**< Decay ID of negative tau lepton decay */
    int m_pprong; /**< Prong of positive tau lepton decay */
    int m_mprong; /**< Prong of negative tau lepton decay */


    ClassDef(TauPairDecay, 1) /**< class definition */

  };


} // end namespace Belle2

