/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

namespace Belle2 {

  namespace HLTUnits {

    /** Maximum number of HLT units used during the experiment. */
    static constexpr unsigned int max_hlt_units = 10;

    /** Location of HLT unit number information. */
    static constexpr char hlt_unit_file[] = "/home/usr/hltdaq/HLT.UnitNumber";

  }

  namespace ExpressRecoUnits {

    /** Maximum number of ExpressReco units used during the experiment. */
    static constexpr unsigned int max_ereco_units = 2;

  }

}
