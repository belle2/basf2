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
    explicit ECLHadronComponentEmissionFunction(TGraph* inHadronComponentEmissionFunction): m_HadronComponentEmissionFunction(
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
     * @param inHadronComponentEmissionFunction hadron component emission function tgraph
     */
    void setHadronComponentEmissionFunction(TGraph* inHadronComponentEmissionFunction) {m_HadronComponentEmissionFunction = inHadronComponentEmissionFunction;};

  private:
    TGraph* m_HadronComponentEmissionFunction; /**< TGraph that holds hadron component emission function*/

    // 1: Initial version
    ClassDef(ECLHadronComponentEmissionFunction, 1); /**< ClassDef */
  };
} // end namespace Belle2
