/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCRECOHIT_H
#define CDCRECOHIT_H

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/ADCCountTranslatorBase.h>
#include <cdc/dataobjects/CDCGeometryTranslatorBase.h>
#include <cdc/dataobjects/DriftTimeTranslatorBase.h>

#include <genfit/GFRecoHitIfc.h>
#include <genfit/GFWireHitPolicy.h>

#include <TMatrixD.h>

//#include <boost/shared_ptr.hpp> // produces trouble for cint because of "Error: void type variable can not be declared "

namespace Belle2 {
  /** @addtogroup cdc_dataobjects
   *  @ingroup dataobjects
   *  @{ CDCRecoHit
   *  @todo CDCRecoHit : once we go to ROOT 6, the pointer should be replaced with shared_ptr.
   *  @}
   */
  /** This class is used to transfer CDC information to the track fit. */
  class CDCRecoHit : public GFRecoHitIfc<GFWireHitPolicy>  {

  public:
    /** Default Constructor for ROOT IO.*/
    CDCRecoHit();

    /** Constructor needed for GenFit RecoHitFactory.
     *
     *  This constructor assumes, that no information from tracking is currently known.
     */
    CDCRecoHit(const CDCHit* cdcHit);

    /** Destructor. */
    ~CDCRecoHit() {}

    /** Creating a copy of this hit.
     *
     *  This function overwrites a function that GFRecoHitIfc inherits from GFRecoHit.
     */
    GFAbsRecoHit* clone();

    /** Projection for the hit ...
     *
     * This function overwrites a function that GFRecoHitIfc inherits from GFRecoHit.
     */
    TMatrixD getHMatrix(const GFAbsTrackRep* stateVector);

    /** Getter for WireID object. */
    WireID getWireID() const {
      return m_wireID;
    }

    /** Setter for the Translators. */
    static void setTranslators(ADCCountTranslatorBase*    const adcCountTranslator,
                               CDCGeometryTranslatorBase* const cdcGeometryTranslator,
                               DriftTimeTranslatorBase*   const driftTimeTranslator);
    /*
    static void setTranslators(boost::shared_ptr<ADCCountTranslatorBase>    const& adcCountTranslator,
                         boost::shared_ptr<CDCGeometryTranslatorBase> const& cdcGeometryTranslator,
                         boost::shared_ptr<DriftTimeTranslatorBase>   const& driftTimeTranslator);
     */

    /** Setter for the update option.
     *
     *  Currently it is strongly recommended to keep this option false.
     */
    static void setUpdate(bool update = false);

    /** Method, that actually interfaces to Genfit.
     *
     *  This method is inherited from the GFAbsRecoHitIfc.
     */
    void getMeasurement(const GFAbsTrackRep*, const GFDetPlane& pl, const TMatrixT<double>&, const TMatrixT<double>&,
                        TMatrixT<double>& m, TMatrixT<double>& V);


  private:
    //--- GENFIT Stuff ----------------------------------------------------------------------------------------------------------
    //NOTE: The endcap positions of the wire is stored in a variable inherited from GFRecoHitIfc<GFWireHitPolicy>.
    /** A parameter for GENFIT. */
    static const int c_nParHitRep = 7;

    /** Holds all elements of H Matrix.  A C-array is the only possibility to set TMatrixD elements with its constructor. */
    static const double c_HMatrixContent[5];

    /** H matrix needed for Genfit. getHMatrix will return this attribute.*/
    static const  TMatrixD c_HMatrix;

    //---------------------------------------------------------------------------------------------------------------------------
    /** Accumulated charge within one drift cell as ADC count. */
    unsigned short m_adcCount;

    /** Accumulated charge within one cell.  UNIT??? */
    float m_charge;

    /** Drift Time as out of CDCHit. */
    short m_driftTime;
    /** Drift Length.
     *
     *  This is basically a cache to avoid recalculation of drift length every time.
     */
    float m_driftLength;
    /** Drift Length Resolution.
     *
     *  Similar issues as with the drif length.
     */
    float m_driftLengthResolution;

    /** Wire Identifier. */
    WireID m_wireID;

    /** Object for ADC Count translation. */
    static ADCCountTranslatorBase*     s_adcCountTranslator;    //! Don't write to ROOT file, as pointer is meaningless, there
    // static boost::shared_ptr<ADCCountTranslatorBase>   s_adcCountTranslator;    //! Don't write to ROOT file, as pointer is meaningless, there

    /** Object for geometry translation. */
    static CDCGeometryTranslatorBase*  s_cdcGeometryTranslator; //! Don't write to ROOT file, as pointer is meaningless, there
    //static boost::shared_ptr<CDCGeometryTranslatorBase> s_cdcGeometryTranslator; //! Don't write to ROOT file, as pointer is meaningless, there

    /** Object for getting drift-length and -resolution. */
    static DriftTimeTranslatorBase*    s_driftTimeTranslator;   //! Don't write to ROOT file, as pointer is meaningless, there
    //static boost::shared_ptr<DriftTimeTranslatorBase>   s_driftTimeTranslator;   //! Don't write to ROOT file, as pointer is meaningless, there

    /** If set to false, the data from the cash is used for the get measurement function.
     *
     *  If set to true, the getMeasurement function tries to update the hit information using the current translators.
     *  As with translator pointers, this is a static variable. So it is sufficient to switch this once to make all CDCRecoHits doing the same.
     */
    static bool s_update; //!                               Don't write to ROOT file, as pointer is meaningless, there

    /** ROOT Macro.*/
    ClassDef(CDCRecoHit, 2);
  };

}
#endif
