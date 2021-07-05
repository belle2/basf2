/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGTIMING_H
#define TRGTIMING_H

#include <framework/datastore/RelationsObject.h>

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
    TRGTiming() :
      m_ID(),
      m_timing() // 2019/07/31 by ytlai
    {;}

    /*! constructor: xxx */
    TRGTiming(unsigned int ID,
              int timing)
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
    int getTiming() const
    {
      return m_timing;
    }

  private:

    // enum {c_PIDDetectorSetSize = 4}; /**< temporary solution for the size */

    // Const::DetectorSet m_detectors;   /**< set of detectors with PID information */

    unsigned int m_ID; /**< bit ID */
    int m_timing; /**< tdc count */

    ClassDef(TRGTiming, 1); /**< ClassDef */

  };


} // end namespace Belle2

#endif
