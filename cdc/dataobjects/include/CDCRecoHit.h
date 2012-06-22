/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCRECOHIT_H
#define CDCRECOHIT_H

#include <framework/gearbox/Unit.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <TVector3.h>
#include <TMatrixD.h>

//base class headers
#include <cdc/dataobjects/CDCHit.h>

//genfit
#include <genfit/GFRecoHitIfc.h>
#include <genfit/GFWireHitPolicy.h>



namespace Belle2 {

  /** This class is an enriched form of the CDCHit class.
   *
   *  It includes all information of CDCHit, plus it includes additional information
   *  from geometry, calibration, GENFIT, etc. <br>
   *  I don't inherit from CDCHit, because inheriting twice from TObject through different routes
   *  is potentially dangerous.
   *
   *  @author <a href="mailto:martin.heck@kit.edu?subject=CDCRecoHit">Martin Heck</a>
   *
   *  @todo So far this class is mostly a stumb...
   */
  class CDCRecoHit : public GFRecoHitIfc<GFWireHitPolicy>  {

  public:

    /** Default Constructor for ROOT IO.*/
    CDCRecoHit();


    /** Constructor needed for GenFit RecoHitFactory.
     *
     */
    CDCRecoHit(const CDCHit* cdcHit);

    /** Destructor. */
    ~CDCRecoHit() {}

    /** Creating a copy of this hit.
     *
     * This function overwrites a function that GFRecoHitIfc inherits from GFRecoHit.
     */
    GFAbsRecoHit* clone();

    /** Projection for the hit ...
     *
     * This function overwrites a function that GFRecoHitIfc inherits from GFRecoHit.
     */
    TMatrixD getHMatrix(const GFAbsTrackRep* stateVector);

    /** Returns the distance of the wire center from the origin (radius of the hit).
     *
     */
    double getRho() const {return m_rho; } ;

    /** Returns the layer number.
     *
     */
    unsigned short int getLayerId() const {return m_layerId; } ;

    /** Returns the wire id.
     *
     */
    unsigned short int getWireId() const {return m_wireId; } ;

    /** Returns the super layer id.
     *
     */
    unsigned short int getSuperLayerId() const {return m_superLayerId; } ;

    /** Returns the sub layer id in the super layer.
     *
     */
    unsigned short int getSubLayerId() const {return m_subLayerId; } ;


  private:

    /** A parameter for GENFIT.
     */
    static const int c_nParHitRep = 7;


    const static double c_HMatrixContent[5];/**< holds all elements of H Matrix. A C array is only possibility to set TMatrixD elements with its constuctor*/
    const static TMatrixD c_HMatrix; /**< H matrix needed for Genfit. getHMatrix will return this attribute*/

    // Next two variables contain wire identification as used in cdc geometry.

    /** The wire has this layer number.
     */
    unsigned short int m_layerId;
    /** Within the layer, this is the wire ID.
     */
    unsigned short int m_wireId;

    //Another useful layer identification for tracking.
    /** Holds the SuperLayer.
     *  A SuperLayer is defined by some consecutive layers, that are parallel.
     */
    unsigned short int m_superLayerId;
    /** Number of Layer within a SuperLayer.
    */
    unsigned short int m_subLayerId;

    /** Drift Time. As CDCHit for the moment holding the DRIFTLENGTH!
     */
    double m_driftTime;

    /** Accumulated charge within one cell.
    */
    double m_charge;

    /** The distance of the wire center from the origin (radius of the hit).
      *
      */
    double m_rho;


    //NOTE: The endcap positions of the wire is stored in a variable inherited from GFRecoHitIfc<GFWireHitPolicy>.

    /** ROOT Macro.*/
    ClassDef(CDCRecoHit, 2);
  };

}
#endif
