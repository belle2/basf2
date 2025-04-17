/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>

#include <iostream>
#include <fstream>
#include <string>
#include <TObject.h>
#include <TString.h>

namespace Belle2 {

  /**
   *   dE/dx wire gain calibration constants
   */

  class CDCDedxBadWires: public TObject {

  public:

    /**
     * Default constructor
     */
    CDCDedxBadWires(): m_badwires() {};

    /**
     * Constructor
     */
    explicit CDCDedxBadWires(const std::vector<double>& badwires): m_badwires(badwires) {};

    /**
     * Destructor
     */
    ~CDCDedxBadWires() {};

    /**
     * Set wire gain
     **/
    void setBadWireStatus(int wire, bool status)
    {
      if (wire < 0 || (unsigned)wire >= m_badwires.size())
        B2WARNING("Asking for a CDC Wire that is not found!");
      else {
        if (status)m_badwires[wire] = 1.0;
        else m_badwires[wire] = 0.0;
      }
    }

    /** Return wire gain
     * @param wire number
     */
    bool getBadWireStatus(int wire) const
    {
      if (wire < 0 || (unsigned)wire >= m_badwires.size()) {
        B2WARNING("Asking for a CDC Wire that is not found!");
      }

      bool status = kFALSE;
      if (m_badwires[wire] == 0)return kTRUE;
      return status;
    }

    /** print all wire numbers
     *
     */
    void printBadWireStatus(bool save = true, std::string sfx = "local") const
    {

      std::ofstream fBadWires;
      if (save)fBadWires.open(Form("cdcdedx_badwires_%s.txt", sfx.c_str()));

      unsigned int badcount = 0, goodcount = 0;
      std::cout << "List of bad wires"  << std::endl;

      for (int jwire = 0; jwire < 14336; jwire++) {
        if (m_badwires.at(jwire) == 0) {
          badcount++;
          std::cout << "\t" << badcount << ": wire # " << jwire  << std::endl;
          if (save)fBadWires << jwire << "\n";
        } else goodcount++;
      }

      if ((badcount + goodcount) != m_badwires.size())
        std::cout << "Wire sum is not 14336" << std::endl;
      else
        std::cout << "\t Total bad wires # " << badcount << std::endl;

      fBadWires.close();
    }

  private:

    std::vector<double> m_badwires; /**< dE/dx gains for each wire */

    ClassDef(CDCDedxBadWires, 1); /**< ClassDef */
  };
} // end namespace Belle2
