/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <map>


namespace Belle2 {


  /** The class for PXD cluster shape index payload
   */

  class PXDClusterShapeIndexPar: public TObject {
  public:
    /** Default constructor */
    PXDClusterShapeIndexPar() {}
    /** Destructor */
    ~PXDClusterShapeIndexPar() {}

    /** Returns feature scalar */
    double computeFeature(int shape, double thetaU, double thetaV) const
    {
      double feature = 0.0;
      /*
      entry, exit = get_entry_exit_index(shape, thetaU, thetaV);

      if (not entry == exit)
        // Feature is the eta value from entry/exit signals
        picked_signals = pick_signals(shape, indexset=[entry,exit]);
        feature = picked_signals[0] / ( picked_signals[0] + picked_signals[1] );
      else:
        //Single digit shape. Feature is the digit charge
        picked_signals = pick_signals(shape, indexset=[entry]);
        feature = picked_signals[0];
      */
      return feature;
    }

    /** Returns label tuple for shape at given angles */
    int getIndex(int shape, double thetaU, double thetaV) const
    {
      /*
      int dlabel = get_short_digital_label(shape, thetaU, thetaV);
      if dlabel in self.getDigitalLabels())
        # Compute feature
        feature = self.computeFeature(shape, thetaU, thetaV)
        # Compute the index
        index = int( np.digitize(feature, self.getPercentiles(dlabel)) )
        # Return label
        return dlabel, index
      else:
        return None
      */
      return 0;
    }

  private:


    ClassDef(PXDClusterShapeIndexPar, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
