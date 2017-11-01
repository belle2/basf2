/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BEAMSPOT_H
#define BEAMSPOT_H

#include <analysis/modules/TreeFitter/InternalParticle.h>
#include <analysis/modules/TreeFitter/ParticleBase.h>
#include <CLHEP/Matrix/Vector.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <framework/dbobjects/BeamParameters.h>
#include <framework/database/DBObjPtr.h>
#include <analysis/ClusterUtility/ClusterUtils.h>

namespace TreeFitter {

  //    InteractionPoint(Particle* particle, bool forceFitAll,
  //                     bool addupsilon) ; //FT: we don't actually have addupsilon in Belle2, this can be trimmed

  /** Class (abstract particle) containing the projection for the beam constraint */
  class InteractionPoint : public ParticleBase {

  public:

    /** Constructor */
    InteractionPoint(Belle2::Particle* particle, bool forceFitAll, int dimension);

    /** Constructor */
    InteractionPoint(Belle2::Particle* daughter);

    virtual ~InteractionPoint();

    /** init particle, used if it has a mother */
    virtual  ErrCode initParticleWithMother(FitParams* fitparams);
    /** init particle, used if it has no mother */
    virtual  ErrCode initMotherlessParticle(FitParams* fitparams);

    /** init the IP "particle"  */
    ErrCode initBeamSpot(Belle2::Particle* particle);

    /** init the IP "particle"  */
    ErrCode initBeamSpot();

    /** init the IP "particle"  */
    ErrCode initBeamSpotCopy();

    /** this is weird  */
    virtual int dim() const { return 3 ; } // (x,y,z)

    /** init parameters  */
    virtual ErrCode initPar1(FitParams*);

    /** does nothing */
    virtual ErrCode initPar2(FitParams*);

    /** init covariance matrix of the constraint  */
    virtual ErrCode initCov(FitParams*) const;

    /** init covariance matrix of the constraint  */
    virtual ErrCode initCovariance(FitParams* fitpar) const;

    /* particle type */
    virtual int type() const { return kInteractionPoint; }

    /**  chi2 of the statevector? */
    virtual double chiSquare(const FitParams* par) const;

    /**  chi2 of the statevector? */
    double  chiSquareCopy(const FitParams* fitparams) const;

    /** the actuall constraint projection  */
    ErrCode projectIPConstraint(const FitParams& fitpar, Projection&) const;


    /** the actuall constraint projection  */
    ErrCode projectIPConstraintCopy(const FitParams& fitparams, Projection& p) const;

    /** the abstract projection  */
    virtual ErrCode projectConstraint(Constraint::Type, const FitParams&, Projection&) const;

    /** the abstract projection  */
    virtual ErrCode projectConstraintCopy(Constraint::Type, const FitParams&, Projection&) const;


    /** adds the IP as a particle to the contraint list  */
    virtual void addToConstraintList(constraintlist& list, int depth) const;

    /** vertex position index in the statevector */
    virtual int posIndex() const { return index(); }

    /**  momentum index in the statevector. no value for beamspot as a particle */
    virtual int momIndex() const { return -1; }

    /**  the lifetime index. the IP does not have a lifetime */
    virtual int tauIndex() const { return -1; }

    /** hast energy  */
    virtual bool hasEnergy() const { return false; }

    /** get name  */
    virtual std::string name() const { return "InteractionPoint"; }

  private:
    /** dimension of the constraint dim=2::IPTube; dim=3::IPSpot  */
    int m_constraintDimension;

    /** vertex position of the IP */
    CLHEP::HepVector m_ipPos;       // interaction point position

    /** covariance of the IP  */
    CLHEP::HepSymMatrix m_ipCov;    // cov matrix

    /** covariance inverse of the IP  */
    CLHEP::HepSymMatrix m_ipCovInv; // inverse of cov matrix

    /** the parameters are initialze elsewhere this is just a pointer to that */
    Belle2::DBObjPtr<Belle2::BeamParameters> m_beamParams;

    /** vertex position of the IP */
    EigenTypes::ColVector m_ipPosVec;

    /** covariance of the IP  */
    EigenTypes::MatrixXd m_ipCovariance;

    /** covariance inverse of the IP  */
    EigenTypes::MatrixXd m_ipCovInverse;
  };
}
#endif //BEAMSPOT_H
