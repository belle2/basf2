/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
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

