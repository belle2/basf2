/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef ECLEVENTINFORMATION_H
#define ECLEVENTINFORMATION_H

#include <TObject.h>

namespace Belle2 {
  /** Store event wide information like background level.
   *
   *  @author <a href="mailto:torben.ferber@physics.ubc.ca?subject=ECLEventInformation">Torben Ferber</a>
   */
  class ECLEventInformation : public TObject {
  public:

    /** Constructor. */
    explicit ECLEventInformation(int backgroundecl = 0)
      : m_backgroundECL(backgroundecl) {}

    /** Background by ECL Setter.
     *
     *  @param backgroundecl Background measured by ECL (out of time digits above threshold).
     */
    void setBackgroundECL(int backgroundecl)
    {
      m_backgroundECL = backgroundecl;
    }


    /** Get Background by ECL
     *
     *  @return Background measured by ECL.
     */
    int getBackgroundECL() const
    {
      return m_backgroundECL;
    }

  private:

    int m_backgroundECL; /**< Background measured by ECL.  */

    // 2: changed name of getters, setters and member variable
    ClassDef(ECLEventInformation, 2); /**< Beam Background measured in situ by ECL. */
  }; //class
} // namespace Belle2
#endif // EVENTINFORMATION
