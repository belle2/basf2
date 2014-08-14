/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLGAMMA_H
#define ECLGAMMA_H

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/DataStore.h>
#include "ecl/dataobjects/ECLShower.h"
#include <math.h>

#include <framework/datastore/RelationsObject.h>
#include <TVector3.h>
#include <TMatrixFSym.h>

namespace Belle2 {

  /*! Class to store ECL Gamma which are selected with criteria as Belle from ECL Shower
   * relation to ECLShower
   * filled in ecl/modules/eclRecGamma/src/ECLGammaReconstructorModule.cc
   */

  class ECLGamma : public RelationsObject {
  public:
    /** default constructor for ROOT */
    ECLGamma() { m_ShowerId = 0;}

    /*! Set Shower ID
     */
    void setShowerId(int ShowerId) { m_ShowerId = ShowerId; }

    /*! Get Shower Id
     * @return Shower Id
     */
    int GetShowerId() const { return m_ShowerId ; }

    /*! Get Shower Id
     * @return Shower Id
     */
    int getShowerId() const { return m_ShowerId ; }

    /*! Get TVector3 Momentum reading ECLShower through ShowerId
     * @return Momentum from ECLShower
     */
    TVector3 getMomentum() const ;

    /*! Get TVector3 Positon assuming gamma comes from IP
     * @return assuming Positon (0,0,0) of Gamma
     */
    TVector3 getPositon() const {
      TVector3 Positon(0., 0., 0.);
      return Positon;
    }

    /*! Get px Momentum reading ECLShower through ShowerId
     * @return px Momentum from ECLShower
     */
    float getPx() const ;

    /*! Get py Momentum reading ECLShower through ShowerId
     * @return py Momentum from ECLShower
     */
    float getPy() const;

    /*! Get pz Momentum reading ECLShower through ShowerId
     * @return pz Momentum from ECLShower
     */
    float getPz() const;

    /*! Get Energy by reading ECLShower through ShowerId
     * @return Momentum from ECLShower
     */
    float getEnergy() const ;

    //! The method to get return TMatrixFSym  4 Momentum Error Matrix
    /*! Get  TMatrixFSym  4 Momentum Error Matrix by reading ECLShower through ShowerId
     * @return Matrix which from convertion from Error E, Theta, Phi of ECLShower
     */
    void getErrorMatrix(TMatrixFSym& m_errorMatrix) const;
    //! The method to get return TMatrixT<float>   4 Momentum Error Matrix
    /*! Get  TMatrixT<float>   4 Momentum Error Matrix by reading ECLShower through ShowerId
     * @return Matrix which from convertion from Error E, Theta, Phi of ECLShower
    */


    void getErrorMatrix7x7(TMatrixFSym& m_errorMatrix) const;
    //! The method to get return TMatrixT<float>   4 Momentum Error Matrix
    /*! Get  TMatrixT<float>   4 Momentum and Vertex Error Matrix by reading ECLShower through ShowerId
     * @return Matrix which from convertion from Error E, Theta, Phi of ECLShower
    */


  private:
    int m_ShowerId;        /**< shower id of this Gamma */
    ClassDef(ECLGamma, 1); /**< ClassDef  */

  };
} // end namespace Belle2

#endif
