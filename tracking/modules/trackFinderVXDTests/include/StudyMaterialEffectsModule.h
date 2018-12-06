#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/pcore/RootMergeable.h>

#include <tracking/spacePointCreation/SpacePoint.h>

#include <svd/dataobjects/SVDTrueHit.h>

#include <TTree.h>
#include <TFile.h>


namespace Belle2 {
  /**
   * StudyMaterialEffectsModule
   *
   */
  class StudyMaterialEffectsModule : public Module {

  public:

    /** Constructor */
    StudyMaterialEffectsModule();

    /** Init the module */
    void initialize() override;
    /** Show progress */
    void event() override;
    /** Don't break the terminal */
    void terminate() override;



    /** takes SVDTrueHit and sensorID to get global position of the hit. If useEntry == true, the entry position is taken, else: exitPosition. */
    B2Vector3D getGlobalPosition(const SVDTrueHit* trueHit, VxdID vxdID, bool useEntry);

    /** takes SVDTrueHit and sensorID to get global momentum of the hit. If useEntry == true, the entry momentum is taken, else: exitMomentum. */
    B2Vector3D getGlobalMomentumVector(const SVDTrueHit* trueHit, VxdID vxdID, bool useEntry);

    /** takes SpacePoint to get the (first) corresponding trueHit connected to the same particle. */
    const SVDTrueHit* getTrueHit(const SpacePoint& aSP);

  protected:

    //Member variables:
    /// Space Points
    StoreArray<Belle2::SpacePoint> m_spacePoints;

    /// ROOT Tree
    StoreObjPtr<RootMergeable<TTree>> m_tree;

    /** a pointer to the file where the Tree shall be stored. */
    TFile* m_file;

    /** residual of hit.phi between layer 3 begin and 3 end. */
    double m_PhiL3L3 = 0;

    /** residual of hit.phi between layer 3 begin and 4 begin. */
    double m_PhiL3L4 = 0;

    /** residual of hit.phi between layer 3 begin and 6 end. */
    double m_PhiL3L6 = 0;

    /** residual of hit.theta between layer 3 begin and 3 end. */
    double m_ThetaL3L3 = 0;

    /** residual of hit.theta between layer 3 begin and 4 begin. */
    double m_ThetaL3L4 = 0;

    /** residual of hit.theta between layer 3 begin and 6 end. */
    double m_ThetaL3L6 = 0;

    /** residual of hit.scatteringAngle (sqrt(theta^2 + phi^2)) between layer 3 begin and 3 end. */
    double m_ScatterAngleL3L3 = 0;

    /** residual of hit.scatteringAngle (sqrt(theta^2 + phi^2)) between layer 3 begin and 4 begin. */
    double m_ScatterAngleL3L4 = 0;

    /** residual of hit.scatteringAngle (sqrt(theta^2 + phi^2)) between layer 3 begin and 6 end. */
    double m_ScatterAngleL3L6 = 0;

    /** residual of hit.scatteringAngle ((outerHit-innerHit).Angle(innerHit))*180/pi between layer 3 begin and 3 end. */
    double m_ScatterAngleGradL3L3 = 0;

    /** residual of hit.scatteringAngle ((outerHit-innerHit).Angle(innerHit))*180/pi between layer 3 begin and 4 begin. */
    double m_ScatterAngleGradL3L4 = 0;

    /** residual of hit.scatteringAngle ((outerHit-innerHit).Angle(innerHit))*180/pi between layer 3 begin and 6 end. */
    double m_ScatterAngleGradL3L6 = 0;

    /** residual of hit.scatteringAngle ((outerHit.Momentum).Angle(innerHit.Momentum))*180/pi between layer 3 begin and 3 end. */
    double m_ScatterAngleV3GradL3L3 = 0;

    /** residual of hit.scatteringAngle (outerHit.Momentum).Angle(innerHit.Momentum))*180/pi between layer 3 begin and 4 begin. */
    double m_ScatterAngleV3GradL3L4 = 0;

    /** residual of hit.scatteringAngle (outerHit.Momentum).Angle(innerHit.Momentum))*180/pi between layer 3 begin and 6 end. */
    double m_ScatterAngleV3GradL3L6 = 0;

    /** residual of hit distance between layer 3 begin and 4 begin. */
    double m_distXY = 0;

    /** residual of hit.momentum between layer 3 begin and 3 end. */
    double m_deltaPL3L3 = 0;

    /** residual of hit.momentum between layer 3 begin and 4 begin. */
    double m_deltaPL3L4 = 0;

    /** residual of hit.momentum between layer 3 begin and 6 end. */
    double m_deltaPL3L6 = 0;

    /// Counter for successfully processed events
    int m_COUNTERsuccessfullEvents = 0;
  };
} // end namespace Belle2

