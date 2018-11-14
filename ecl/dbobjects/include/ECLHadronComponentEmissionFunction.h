/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Hadron Component Emission Function for hadron pulse shape simulations  *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Savino Longo                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <TGraph.h>

namespace Belle2 {

  /**
   * Hadron Component Emission Function for hadron pulse shape simulations
   */

  class ECLHadronComponentEmissionFunction: public TObject {
  public:

    /**
     * Default constructor
     */
    ECLHadronComponentEmissionFunction(): m_HadronComponentEmissionFunction(0) {};

    /**
     * Constructor
     */
    ECLHadronComponentEmissionFunction(TGraph* inHadronComponentEmissionFunction): m_HadronComponentEmissionFunction(
        inHadronComponentEmissionFunction) {};

    /**
     * Destructor
     */
    ~ECLHadronComponentEmissionFunction() {};

    /** Return hadron component emission function tgraph
     * @return hadron component emission function tgraph
     */
    TGraph* getHadronComponentEmissionFunction() const {return m_HadronComponentEmissionFunction;};

    /** Set hadron component emission function tGraph
     * @param hadron component emission function tgraph
     */
    void setHadronComponentEmissionFunction(TGraph* inHadronComponentEmissionFunction) {m_HadronComponentEmissionFunction = inHadronComponentEmissionFunction;};

  private:
    TGraph* m_HadronComponentEmissionFunction; /**< TGraph that holds hadron component emission function*/

    // 1: Initial version
    ClassDef(ECLHadronComponentEmissionFunction, 1); /**< ClassDef */
  };
} // end namespace Belle2
