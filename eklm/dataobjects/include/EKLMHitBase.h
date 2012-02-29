/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMHITBASE_H
#define EKLMHITBASE_H

#include <sys/types.h>
#include <TObject.h>
#include <TVector3.h>
#include  <string>

namespace Belle2 {

  /**
   * Base Hit reconstruction class. inherit HitBase class from TObject
   *  to make all hits storable
   */
  class EKLMHitBase: public TObject  {

  public:

    /**
     *  Constructor
     */
    EKLMHitBase();

    /**
     *  Constructor
     */

    EKLMHitBase(int Endcap, int Layer, int Sector, int PDG, double Time, double EDep,  TVector3 GlobalPosition, TVector3 LocalPosition);

    /**
     * Destructor
     */
    ~EKLMHitBase() {};

    /**
     * Get endcap number.
     */
    int getEndcap() const;

    /**
     * Set endcap number.
     */
    void setEndcap(int Endcap);

    /**
     * Get layer number.
     */
    int getLayer() const;

    /**
     * Set layer number.
     */
    void setLayer(int nLayer);

    /**
     * Get sector number.
     */
    int getSector() const;

    /**
     * Set sector number.
     */
    void setSector(int nSector);

    /**
     * Get EDep
     */
    double getEDep() const;

    /**
     * set EDep.
     */
    void setEDep(double eDep);

    /**
     * increase EDep.
     */
    void increaseEDep(double deltaEDep);

    /**
     * Get hit time
     */
    double getTime() const;

    /**
     * Set hit time.
     */
    void setTime(double time);

    /**
     * Get the lund code of the (leading) particle
     */
    int getPDG() const ;


    /**
     *  Set the lund code of the (leading) particle
     */
    void setPDG(int);


    /**
     * Get global position of the particle hit
     */
    const TVector3* getPosition() const;

    /**
     * Set global position of the particle hit
     */
    void setPosition(TVector3& position);

    /**
     * Set global position of the particle hit
     */
    void setPosition(const TVector3* position);


    /**
     * Get global position of the particle hit
     */
    const TVector3* getLocalPosition() const;

    /**
     * Set global position of the particle hit
     */
    void setLocalPosition(TVector3& position);

    /**
     * Set global position of the particle hit
     */
    void setLocalPosition(const TVector3* position);


  protected:

    /**
     * PDG code of the (leading) particle
     */
    int m_PDG;

    /**
     * time of the hit
     */
    double m_Time;

    /**
     * Energy deposition
     */
    double m_EDep;

    /**
     * global position of the hit
     */
    TVector3 m_GlobalPosition;

    /**
     * local position of the hit
     */
    TVector3 m_LocalPosition;



  private:


    /**
     * Number of endcap.
     */
    int m_Endcap;

    /**
     * Number of layer.
     */
    int m_Layer;


    /**
     * Number of sector.
     */
    int m_Sector;

    /**
     * Needed to make objects storable.
     */
    ClassDef(Belle2::EKLMHitBase, 1);



  };




} // end of namespace Belle2

#endif //EKLMHITBASE_H
