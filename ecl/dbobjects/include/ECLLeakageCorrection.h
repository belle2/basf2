/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Leakage corrections for ECL showers (N1).                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *               Alon Hershenhorn (hershen@physics.ubc.ca)                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLLEAKAGECORRECTION_H
#define ECLLEAKAGECORRECTION_H
#include <TObject.h>
#include <TNtuple.h>

namespace Belle2 {

  /**
   * Corrections to the second moment shower shape
   */

  class ECLLeakageCorrection: public TObject {
  public:

    /**
     * Default constructor
     */
    //ECLLeakageCorrection(): m_helper(), m_correction() {};

    /**
     * Constructor
     */
    ECLLeakageCorrection(TNtuple helper, TNtuple correction)
    {
      m_helper = (TNtuple*) helper.CloneTree(0);
      m_correction = (TNtuple*) correction.CloneTree(0);
    }

    /**
     * Destructor
     */
    ~ECLLeakageCorrection() {};

    /** Return helper ntuple
     * @return ntuple with helper information
     */
    TNtuple* getHelperNtuple() const {return m_helper;};

    /** Return correction ntuple
     * @return ntuple with correction information
     */
    TNtuple* getCorrectionNtuple() const {return m_correction;};

    /** Set helper ntuple
     * @param helper ntuple
     */
    void setHelperNtuple(TNtuple helper) {m_helper = (TNtuple*) helper.CloneTree(0);};

    /** Set correction ntuple
     * @param correction ntuple
     */
    void setCorrectionNtuple(TNtuple correction) {m_correction = (TNtuple*) correction.CloneTree(0);};

  private:
    TNtuple* m_helper; //<** TNtuple that holds helper variables like bin boundaries */
    TNtuple* m_correction; //<** TNtuple that holds corrections  */

    // 1: Initial version
    ClassDef(ECLLeakageCorrection, 1); /**< ClassDef */
  };
} // end namespace Belle2
#endif
