/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
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


