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

#ifndef __BASEJETPAIRING_H
#define __BASEJETPAIRING_H


namespace Belle2 {
  namespace OrcaKinFit {

    class JetFitObject;

//  Class BaseConstraint:
/// Abstract base class for jet pairings
    /**
     *
     * Author: Jenny List, Benno List
     * Last update: $Date: 2008/02/12 10:19:05 $
     *          by: $Author: blist $
     *
     */
    class BaseJetPairing {
    public:
      virtual ~BaseJetPairing() {};

      // setters
      virtual void reset() {iperm = 0;};

      // getters
      virtual int getNPerm() const = 0;

      // does the job
      virtual int nextPermutation(JetFitObject* permObjects[]) = 0;

    protected:
      int iperm;

    };

  }// end OrcaKinFit namespace
} // end Belle2 namespace


#endif // __BASEJETPAIRING_H

