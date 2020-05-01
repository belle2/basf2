/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* ROOT headers. */
#include <TObject.h>

namespace Belle2 {

  /**
   * DBObject containing KLM time window parameters used in KLMReconstructor module.
   * Unit is ns.
   */
  class KLMTimeWindow : public TObject {
  public:

    /**
    * Default constructor
    */
    KLMTimeWindow():
      m_CoincidenceWindow(0),
      m_PromptTime(0),
      m_PromptWindow(0)
    {};

    /**
    * Constructor
    */
    KLMTimeWindow(double coincidenceWindow, double promptTime, double promptWindow)
    {
      m_CoincidenceWindow = coincidenceWindow;
      m_PromptTime = promptTime;
      m_PromptWindow = promptWindow;
    };

    /**
    * Destructor
    */
    ~KLMTimeWindow() {};

    /**
     * Set the coincidence window.
     * @param[in] coincidenceWindow Coincidence window.
     */
    void setCoincidenceWindow(double coincidenceWindow) { m_CoincidenceWindow = coincidenceWindow; }

    /**
     * Set the prompt Time.
     * @param[in] promptTime Prompt time.
     */
    void setPromptTime(double promptTime) { m_PromptTime = promptTime; }

    /**
     * Set the time window.
     * @param[in] timeWindow Time window.
     */
    void setPromptWindow(double promptWindow) { m_PromptWindow = promptWindow; }

    /**
    * Get the coincidence window.
    */
    double getCoincidenceWindow() const {return m_CoincidenceWindow;}

    /**
     * Get the prompt time.
     */
    double getPromptTime() const { return m_PromptTime; }

    /**
    * Get the time window.
    */
    double getPromptWindow() const { return m_PromptWindow; }

  private:

    /**
     * Half-width of the time coincidence window used to create a 2D hit from 1D digits/hits.
     */
    double m_CoincidenceWindow;

    /**
     * Nominal time of prompt BKLMHit2ds.
     */
    double m_PromptTime;

    /**
     * Half-width of the time window relative to the prompt time for BKLMHit2ds.
     */
    double m_PromptWindow;

    /**
     * Class version.
     */
    ClassDef(KLMTimeWindow, 1);

  };

} // end namespace Belle2
