/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//
// $Id: B2BIIFixMdst_brecon.cc 9932 2006-11-12 14:26:53Z katayama $
//
// $Log$
//
// Revision 2.0 2015/03/11 tkeck
// Conversion to Belle II
//
// Revision 1.1  2002/03/31 06:54:40  katayama
// fix_pi0
//
//

#include <b2bii/modules/B2BIIMdstInput/B2BIIFixMdstModule.h>
#include "belle_legacy/panther/panther.h"
#include "belle_legacy/tables/brecon.h"
#include "belle_legacy/tables/mdst.h"
#include "belle_legacy/tables/belletdf.h"

namespace Belle2 {


// fix relation gamma<->pi0 in brecon table
// momenta will NOT be modified
  void B2BIIFixMdstModule::fix_pi0_brecon(void)
  {
    Belle::Brecon_Manager& breconmgr = Belle::Brecon_Manager::get_manager();
    if (!breconmgr.count()) return;
    Belle::Mdst_pi0_Manager& pi0mgr = Belle::Mdst_pi0_Manager::get_manager();

    for (std::vector<Belle::Brecon>::iterator it = breconmgr.begin();
         it != breconmgr.end(); ++it) {

      // select the table of pi0->gamma gamma
      if (std::abs((*it).pcode()) != 111) continue;
      const int da_first((*it).daFirst());
      const int da_last((*it).daLast());
      if (da_first < 1 || da_last < 1) continue;
      if (da_last - da_first != 1) continue;
      const Belle::Brecon& gamma1(breconmgr[da_first - 1]);
      const Belle::Brecon& gamma2(breconmgr[da_last - 1]);
      if (gamma1.stable() != 2 || gamma2.stable() != 2) continue;

      // find corresponding Mdst_pi0
      int pi0_ID(-1);
      const Belle::Panther_ID g1(gamma1.idmdst());
      const Belle::Panther_ID g2(gamma2.idmdst());
      for (std::vector<Belle::Mdst_pi0>::const_iterator it2 = pi0mgr.begin();
           it2 != pi0mgr.end(); ++it2) {
        const Belle::Panther_ID da1((*it2).gamma_ID(0));
        const Belle::Panther_ID da2((*it2).gamma_ID(1));
        if ((g1 == da1 && g2 == da2) || (g1 == da2 && g2 == da1)) {
          pi0_ID = (int)(*it2).get_ID();
          break;
        }
      }

      // modify brecon table
      switch (pi0_ID) {
        case -1: // corresponding Mdst_pi0 is not found after make_pi0
          (*it).stable(0);
          (*it).idmdst(0);
          break;
        default:
          (*it).stable(4);
          (*it).idmdst(pi0_ID);
          break;
      }
    }
  }
} // namespace Belle
