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
    ECLEventInformation(int backgroundbyecl = 0)
      : m_backgroundByECL(backgroundbyecl) {}


    /** Background by ECL Setter.
     *
     *  @param backgroundbyecl Background measured by ECL (out of time digits above threshold).
     */
    void setBackgroundByECL(int backgroundbyecl)
    {
      m_backgroundByECL = backgroundbyecl;
    }


    /** Get Background by ECL
     *
     *  @return Background measured by ECL.
     */
    int getBackgroundByECL() const
    {
      return m_backgroundByECL;
    }

  private:

    int m_backgroundByECL; /**< Background measured by ECL.  */

    ClassDef(ECLEventInformation, 1); /**< Beam Background measured in situ by ECL. */
  }; //class
} // namespace Belle2
#endif // EVENTINFORMATION
