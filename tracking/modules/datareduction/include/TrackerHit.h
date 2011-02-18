// -*- C++ -*-
#ifndef TRACKERHIT_H
#define TRACKERHIT_H

#include <string>


namespace Belle2 {

  class TrackerHit {

  public:
    // C'tor
    TrackerHit() ;

    /// Destructor.
    virtual ~TrackerHit() ;


    /** The hit  position in [mm].
     */
    virtual const double* getPosition() const ;

    /**Covariance of the position (x,y,z)
     */
//   virtual const EVENT::FloatVec & getCovMatrix() const ;

    /** The dE/dx of the hit in [GeV/mm].
     */
    virtual float getdEdx() const ;

    /** The  time of the hit in [ns]. Is this needed ?
     */
    virtual float getTime() const ;

//     /**Type of raw data hit, either one of<br>
//      * LCIO::TPCHIT<br>
//      * LCIO::SIMTRACKERHIT<br>
//      */
//     virtual const std::string & getType() const ;

    /** Type of hit. Mapping of integer types to type names
     * through collection parameters "TrackerHitTypeNames"
     * and "TrackerHitTypeValues".
     */
    virtual int getType() const ;


    /** The raw data hits.
     * Check getType() to get actual data type.
     */
//    virtual const EVENT::LCObjectVec & getRawHits() const ;


    /** Use to manipulate the raw hits.
     */
//   virtual EVENT::LCObjectVec & rawHits() ;


    // setters
    void setType(int type) ;
    void setPosition(double pos[3]) ;
//    void setCovMatrix( const EVENT::FloatVec& cov );
//    void setCovMatrix( float cov[TRKHITNCOVMATRIX]  );
    void setdEdx(float dedx) ;
    void setTime(float t) ;



  protected:

    int _type ;
    double _pos[3] ;
//    EVENT::FloatVec _cov ;
    float _dEdx ;
    float _time ;
//    EVENT::LCObjectVec _rawHits ;


  }; // class
}
#endif /* ifndef _TRACKERHIT_H */
