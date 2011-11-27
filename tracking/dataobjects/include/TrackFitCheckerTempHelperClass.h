/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Moritz Nadler                                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

/* created by Moritz Nadler used by the TrackFitCheckerModule will be repaced
 * with something better in the future
 */

#ifndef TrackFitCheckerTempHelperClass_H
#define TrackFitCheckerTempHelperClass_H

#include <TObject.h>
namespace Belle2 {

  /**
  */

  class TrackFitCheckerTempHelperClass : public TObject {
  public:

    /**  Empty constructor.
    *
    *  This constructor is needed for I/O purposes.
    */
    TrackFitCheckerTempHelperClass() {

    };


    /** Destructor.
    *
    *  As no pointers are used, there is nothing special done here.
    */
    ~TrackFitCheckerTempHelperClass() {}




    double chi2tot_fu;
    double chi2tot_fp;
    double chi2tot_bu;
    double chi2tot_bp;
    double pValue_fu;
    double pValue_fp;
    double pValue_bu;
    double pValue_bp;

    double zVertexPosX;
    double zVertexPosY;
    double zVertexPosZ;
    double zVertexMomX;
    double zVertexMomY;
    double zVertexMomZ;

    double vertexPosX;
    double vertexPosY;
    double vertexPosZ;
    double vertexMomX;
    double vertexMomY;
    double vertexMomZ;
    /** Needed for ROOT purposes. This Macro makes a ROOT object from TrackFitCheckerTempHelperClass. */
    ClassDef(TrackFitCheckerTempHelperClass, 1);
  }; //class
} // namespace Belle2
#endif
