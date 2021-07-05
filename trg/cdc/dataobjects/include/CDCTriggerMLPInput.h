/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <TObject.h>

namespace Belle2 {

  /**
   * Class to hold some intermediate information for the monitoring of the neurotrigger.
   */
  class CDCTriggerMLPInput : public TObject {
  public:
    /** default constructor */
    CDCTriggerMLPInput() {};

    /** explicit constructor */
    CDCTriggerMLPInput(std::vector<float>& input, unsigned sector):
      m_input(input), m_sector(sector)
    {};

    /** destructor, empty because we don't allocate memory explicitly. */
    ~CDCTriggerMLPInput() { };

    /** get the input vector */
    std::vector<float> getInput()
    {
      return m_input;
    }
    /** get the sector index */
    unsigned getSector()
    {
      return m_sector;
    }

  protected:
    /** input vector to go into the MLP */
    std::vector<float> m_input;
    /** sector index to identify the MLP */
    unsigned m_sector;

    //! Needed to make the ROOT object storable
    ClassDef(CDCTriggerMLPInput, 1);
  };

}

