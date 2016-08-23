//#include <analysis/dataobjects/Particle.h>

#include <analysis/modules/TreeFitter/RecoComposite.h>
#include <analysis/modules/TreeFitter/FitParams.h>

#include <TMatrixFSym.h>
//using std::cout;
//using std::endl;

namespace TreeFitter {
  extern int vtxverbose ;

  //FT: This is used for fitted/reconstructed composites, not for internal particles! WIP
  RecoComposite::RecoComposite(Particle* particle, const ParticleBase* mother)
    : ParticleBase(particle, mother), m_m(), m_matrixV(), m_hasEnergy(true)
  {
    //    bool massconstraint = particle && particle->constraint(BtaConstraint::Mass) ;//FT: this only makes sense within BaBar, redo
    //    m_hasEnergy = !massconstraint ;                                              //
    updCache() ;
  }

  void RecoComposite::updCache()//FT had to split up all calls to "btafitparams"
  {
    // cache par7 (x,y,z,px,py,pz,E) cov7
    // the simplest is
    //    const FitParams particlefitparams = particle()->fitParams() ;
    //    HepPoint pos = particlefitparams.pos() ;
    TVector3 pos = particle()->getVertex();
    //    HepLorentzVector mom = particlefitparams.p4() ;
    TVector3 mom = particle()->getMomentum();
    double energy = particle()->getEnergy();
    m_m = HepVector(dimM()); //FT: note that Belle2 variables are ordered (p,E,x) while the treefitter internal logic uses (x,p,E)
    m_m(1) = pos.X() ;
    m_m(2) = pos.Y() ;
    m_m(3) = pos.Z() ;
    m_m(4) = mom.X() ;
    m_m(5) = mom.Y() ;
    m_m(6) = mom.Z() ;
    m_m(7) = energy ;

    //FT: this part will need to be reviewed, if ever needed;
    //    m_matrixV = particlefitparams.cov7().sub(1,dimM()) ; // so either 7 or 6, depending on mass constraint
    TMatrixFSym cov7in = particle()->getMomentumVertexErrorMatrix(); //this is (p,E,x)
    HepSymMatrix cov7out(7, 0); //this has to be (x,p,E)
    for (int row = 1; row <= 4; ++row) { //first the p,E block
      for (int col = 1; col <= row; ++col) {
        cov7out(3 + row, 3 + col) = cov7in[row - 1][col - 1];
      }
    }
    for (int row = 1; row <= 3; ++row) { //then the x block
      for (int col = 1; col <= row; ++col) {
        cov7out(row, col) = cov7in[3 + row][3 + col];
      }
    }
    for (int row = 1; row <= 4; ++row) { //and the diagonal
      for (int col = 1; col <= 3; ++col) {
        cov7out(col, 3 + row) = cov7in[row - 1][3 + col];
      }
    }
    //FT this can be optimized later into a single loop
    m_matrixV = cov7out.sub(1, dimM()) ; // so either 7 or 6, depending on mass constraint
    if (vtxverbose >= 4) {
      std::cout << "cov matrix of external candidate: " << name().c_str()
                << " " << dimM() << " " << m_matrixV << std::endl ;
    }
  }

  RecoComposite::~RecoComposite() {}

  ErrCode RecoComposite::initPar1(FitParams* fitparams)
  {
    int posindex = posIndex() ;
    int momindex = momIndex() ;

    //quick map for parameters
    int indexmap[7]  ;
    for (int i = 0; i < 3; ++i) indexmap[i]   = posindex + i ;
    for (int i = 0; i < 4; ++i) indexmap[i + 3] = momindex + i ;
    // copy the 'measurement'
    for (int row = 1; row <= dimM(); ++row)
      fitparams->par()(indexmap[row - 1] + 1) = m_m(row) ;

    return ErrCode::success ;
  }

  ErrCode RecoComposite::initPar2(FitParams* fitparams)
  {
    // call default lifetime initialization
    return initTau(fitparams) ;
  }

  ErrCode RecoComposite::projectRecoComposite(const FitParams& fitparams, Projection& p) const
  {
    int posindex = posIndex() ;
    int momindex = momIndex() ;

    // quick map for parameters
    int indexmap[7]  ;
    for (int i = 0; i < 3; ++i) indexmap[i]   = posindex + i ;
    for (int i = 0; i < 4; ++i) indexmap[i + 3] = momindex + i ;
    for (int row = 1; row <= dimM(); ++row) {
      p.r(row)                   = fitparams.par()(indexmap[row - 1] + 1) - m_m(row) ;
      p.H(row, indexmap[row - 1] + 1) = 1 ;
      for (int col = 1; col <= row; ++col)
        p.Vfast(row, col) = m_matrixV.fast(row, col) ;
    }
    return ErrCode::success ;
  }

  ErrCode RecoComposite::projectConstraint(Constraint::Type type, const FitParams& fitparams, Projection& p) const
  {
    //FT: this whole function seems to rely on properties that we don't have available in basf2?
    ErrCode status ;
    switch (type) {
      case Constraint::composite:
        status |= projectRecoComposite(fitparams, p) ;
        break ;
      case Constraint::geometric:
        status |= projectGeoConstraint(fitparams, p) ;
        break ;
      default:
        status |= ParticleBase::projectConstraint(type, fitparams, p) ;
    }
    return status ;
  }

  double RecoComposite::chiSquare(const FitParams* fitparams) const
  {
    Projection p(fitparams->dim(), dimM()) ;
    projectRecoComposite(*fitparams, p) ;
    return p.chiSquare() ;
  }
}
