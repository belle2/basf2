/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <TGraph.h>

namespace Belle2 {

  /**
   * Corrections to the second moment shower shape
   */

  class ECLShowerShapeSecondMomentCorrection: public TObject {
  public:

    /**
     * Default constructor
     */
    ECLShowerShapeSecondMomentCorrection(): m_hypothesisId(0), m_type(0), m_correction(0) {};

    /**
     * Constructor
     */
    ECLShowerShapeSecondMomentCorrection(int hypothesisid, int type, TGraph correction): m_hypothesisId(hypothesisid), m_type(type),
      m_correction(correction) {};

    /**
     * Destructor
     */
    ~ECLShowerShapeSecondMomentCorrection() {};

    /** Return hypothesis Id
     * @return hypothesis id
     */
    int getHypothesisId() const {return m_hypothesisId; };

    /** Return type
     * @return type
     */
    int getType() const {return m_type; };

    /** Return corrections tgraph
     * @return corrections tgraph
     */
    TGraph getCorrection() const {return m_correction; };

    /** Set hypothesis id
     * @param hypothesisid hypothesis id
     */
    void setHypothesisId(int hypothesisid) {m_hypothesisId = hypothesisid; };

    /** Set type
     * @param type
     */
    void setType(int type) {m_type = type; };

    /** Set correction TGraph
     * @param correction tgraph
     */
    void setCorrection(TGraph correction) {m_correction = correction;};

  private:
    int m_hypothesisId; /**< Hypthesis Id */
    int m_type; /**< Type (e.g. theta) */
    TGraph m_correction; /**< TGraph that holds corrections*/

    // 1: Initial version
    ClassDef(ECLShowerShapeSecondMomentCorrection, 1); /**< ClassDef */
  };
} // end namespace Belle2
