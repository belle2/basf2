#include <tracking/modules/datareduction/TrackerHit.h>
#include <framework/logging/Logger.h>


using namespace Belle2;
using namespace std;


TrackerHit::TrackerHit() :
    _type(0),
    _dEdx(0),
    _time(0)
{
  _pos[0] = 0. ;
  _pos[1] = 0. ;
  _pos[2] = 0. ;

  // _cov.resize( TRKHITNCOVMATRIX ) ;
  //     for(int i=0;i<TRKHITNCOVMATRIX;i++){
  //       _cov.push_back(0.0) ;
  //     }
}

TrackerHit::~TrackerHit()
{
}

const double* TrackerHit::getPosition() const {  return _pos ; }

/*
const FloatVec & TrackerHitImpl::getCovMatrix() const {
  return _cov ;
}
*/
float TrackerHit::getdEdx() const { return _dEdx ; }

float TrackerHit::getTime() const { return _time ; }

/*
  const EVENT::LCObjectVec & TrackerHitImpl::getRawHits() const {
    return _rawHits ;
  }

  EVENT::LCObjectVec & TrackerHitImpl::rawHits() {
    checkAccess("TrackerHitImpl::rawHits") ;
    return _rawHits ;
  }
*/
//   const std::string & TrackerHitImpl::getType() const {
//     static std::string tpcHitType( LCIO::TPCHIT ) ;
//     static std::string unknown( "Unknown" ) ;
//     TPCHit* tpchit = dynamic_cast<TPCHit*>( _rawHit ) ;
//     if( tpchit != 0 )
//       return tpcHitType ;
//     else
//       return unknown ;
//   }

int TrackerHit::getType() const
{
  return _type ;
}


void TrackerHit::setType(int type)
{
  _type = type ;
}

void TrackerHit::setPosition(double pos[3])
{
  //B2DEBUG(99, "Pos: " << pos[0] << "  " << pos[1] << "  " << pos[2]);
  _pos[0] = pos[0] ;
  _pos[1] = pos[1] ;
  _pos[2] = pos[2] ;
}

void TrackerHit::setdEdx(float dedx)
{
  _dEdx = dedx ;
}

void TrackerHit::setTime(float t)
{
  _time = t ;
}
/*
  void TrackerHitImpl::setCovMatrix( const FloatVec& cov ){
    for(int i=0;i<TRKHITNCOVMATRIX;i++){
      _cov[i] = cov[i] ;
    }
  }
  void TrackerHitImpl::setCovMatrix( float cov[TRKHITNCOVMATRIX]  ){
     for(int i=0;i<TRKHITNCOVMATRIX;i++){
      _cov[i] = cov[i] ;
    }
  }
*/

