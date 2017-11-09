/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMTIMEWINDOW_DB_H
#define BKLMTIMEWINDOW_DB_H

#include <TObject.h>
#include <string>
#include <cmath>
#include <TROOT.h>
#include <TClass.h>

namespace Belle2 {

  /**
   * The Class for BKLM time window requirement (unit in ns)
   */

  class BKLMTimeWindow: public TObject {
  public:

    /**
    * Default constructor
    */
    BKLMTimeWindow(): m_CoincidenceWindow(0), m_PromptTime(0), m_PromptWindow(0) {};

    /**
    * Constructor
    */
    BKLMTimeWindow(double coincidenceWindow, double promptTime, double promptWindow)
    {
      m_CoincidenceWindow = coincidenceWindow;
      m_PromptTime = promptTime;
      m_PromptWindow = promptWindow;
    };

    /**
    * Destructor
    */
    ~BKLMTimeWindow() {};

    /**
    * Get the Coincidence Window
    */
    double getCoincidenceWindow() {return m_CoincidenceWindow;}

    /**
     * Get the Prompt Time
     */
    double getPromptTime() { return m_PromptTime; }

    /**
    * Get the time window
    */
    double getPromptWindow() { return m_PromptWindow; }

    /**
     * Set the Coincidence Window
     */
    void setCoincidenceWindow(double coincidenceWindow) { m_CoincidenceWindow = coincidenceWindow; }

    /**
     * Set the prompt Time
     */
    void setPromptTime(double promptTime) { m_PromptTime = promptTime; }

    /**
     * Set the time window
     */
    void setPromptWindow(double promptWindow) { m_PromptWindow = promptWindow; }


  private:

    /**
     * coincidence Window of phi and z strips (ns)
     */
    double m_CoincidenceWindow;

    /**
     * prompt Time (ns)
     */
    double m_PromptTime;

    /**
     * time window (ns)
     */
    double m_PromptWindow;

    /**
     * ClassDef, must be the last term before the closing {}
     */
    ClassDef(BKLMTimeWindow, 1);

  };

} // end namespace Belle2

#endif
