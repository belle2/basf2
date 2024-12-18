/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * Forked from https://github.com/iLCSoft/MarlinKinfit                    *
 *                                                                        *
 * Further information about the fit engine and the user interface        *
 * provided in MarlinKinfit can be found at                               *
 * https://www.desy.de/~blist/kinfit/doc/html/                            *
 * and in the LCNotes LC-TOOL-2009-001 and LC-TOOL-2009-004 available     *
 * from http://www-flc.desy.de/lcnotes/                                   *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef BASEDEFS_HH
#define BASEDEFS_HH

namespace Belle2 {
  namespace OrcaKinFit {

    class BaseDefs {
    public:

      // define labels for bases (sets of intermediate variables)
      enum { VARBASIS_EPXYZ = 0, VARBASIS_VXYZ, VARBASIS_TRKNORMAL, NMETASET };

      // max # of variables in the above bases
      enum {MAXINTERVARS = 4};

      // maximum number of parameters for a fit object
      enum {MAXPAR = 10};

      // this is used to store how many variables in each base (should be <= maxInter)
      static const int nMetaVars[NMETASET];

    };

  }// end OrcaKinFit namespace
} // end Belle2 namespace

#endif


