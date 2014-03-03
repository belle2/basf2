/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TRGSUMMARY_H
#define TRGSUMMARY_H

#include <framework/datastore/RelationsObject.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>


namespace Belle2 {

  /** \addtogroup dataobjects
   * @{
   */

  class TRGSummary : public RelationsObject {

  private:

    static const int c_Version = 0;

  public:

    /*! constructor: xxx */
    TRGSummary(unsigned int inputBits[10],
               unsigned int ftdlBits[10],
               unsigned int psnmBits[10],
               unsigned int timTypeBits) {
      for (int i = 0; i < 10; i++) {
        m_inputBits[i] = inputBits[i];
        m_ftdlBits[i] = ftdlBits[i];
        m_psnmBits[i] = psnmBits[i];
      }
      m_timTypeBits = timTypeBits;
    }

    /*! default constructor: xxx */
    TRGSummary() {;}

    /** Destructor.
     */
    ~TRGSummary() {}

    /*! setter
     * @param xxx explanation
     */
    void setTRGSummary() {;}

    /*! get input bits
     * @param i index: 0, 1, 2 for bit 0-31, 32-63, 64-95, respectively.
     * @return     input bits
     */
    unsigned int getInputBits(const unsigned i) const {
      return m_inputBits[i];
    }

    /*! get ftdl bits
     * @param i index: 0, 1, 2 for bit 0-31, 32-63, 64-95, respectively.
     * @return     ftdl bits
     */
    unsigned int getFtdlBits(const unsigned i) const {
      return m_ftdlBits[i];
    }

    /*! get psnm bits
     * @param i index: 0, 1, 2 for bit 0-31, 32-63, 64-95, respectively.
     * @return     psnm bits
     */
    unsigned int getPsnmBits(const unsigned i) const {
      return m_psnmBits[i];
    }

    /*! get timing source information
     * @return     timing source bits
     */
    unsigned int getTimTypeBits() const {
      return m_timTypeBits;
    }

  private:

    // enum TimingSource {c_BPID, c_ECL, c_CDC, c_GDL, c_SPARE}; /**< */
    // enum TriggerType {c_Physics, c_Random, c_Calibration, c_SPARE}; /**< */
    // enum {c_PIDDetectorSetSize = 4}; /**< temporary solution for the size */

    // Const::DetectorSet m_detectors;   /**< set of detectors with PID information */

    unsigned int m_inputBits[10]; /**< input bits from subdetectors */
    unsigned int m_ftdlBits[10]; /**< ftdl bits */
    unsigned int m_psnmBits[10]; /**< psnm bits */
    unsigned int m_timTypeBits; /**< timing source bits */

    ClassDef(TRGSummary, 1); /**< ClassDef */

  };

  /** @}*/

} // end namespace Belle2

#endif
