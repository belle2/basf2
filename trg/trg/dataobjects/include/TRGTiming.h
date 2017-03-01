/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TRGTIMING_H
#define TRGTIMING_H

#include <framework/datastore/RelationsObject.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>


namespace Belle2 {

  /**
   * Trigger Timing Information for a fired bit.
   */
  class TRGTiming : public RelationsObject {

  private:

    /**
     * version of this code
     */
    static const int c_Version = 0;

  public:

    /*! default constructor: xxx */
    TRGTiming() {;}

    /*! constructor: xxx */
    TRGTiming(unsigned int ID,
              unsigned int timing)
    {
      m_ID = ID;
      m_timing = timing;
    }

    /** Destructor.
     */
    ~TRGTiming() {}

    /*! setter
     * @param xxx explanation
     */
    void setTRGTiming() {;}

    /*! get trigger bit ID
     * @return     trigger bit ID
     */
    unsigned int getID() const
    {
      return m_ID;
    }

    /*! get trigger timing
     * @return    trigger timing
     */
    unsigned int getTiming() const
    {
      return m_timing;
    }

  private:

    // enum {c_PIDDetectorSetSize = 4}; /**< temporary solution for the size */

    // Const::DetectorSet m_detectors;   /**< set of detectors with PID information */

    unsigned int m_ID; /**< bit ID */
    unsigned int m_timing; /**< tdc count */

    ClassDef(TRGTiming, 1); /**< ClassDef */

  };


} // end namespace Belle2

#endif
