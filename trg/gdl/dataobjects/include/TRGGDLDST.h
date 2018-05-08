//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGGDLDST.h
// Section  : TRG GDL
// Owner    :
// Email    :
//-----------------------------------------------------------
// Description : A class to save TRG GDL information of event.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------
#pragma once
#include <mdst/dataobjects/TRGSummary.h>
#include <TObject.h>
#include <TVector3.h>
namespace Belle2 {

  class TRGGDLDST : public TObject {
  public:

    /**! The Class title*/
    ClassDef(TRGGDLDST, 1); /*< the class title */

    TRGGDLDST() {}

    ~TRGGDLDST() {}

    /** set timestamp when GDL receives timing sources*/
    void setTimsrcGdlTime(int top, int ecl, int cdc)
    {
      m_timsrc_gdl_time[2] = top;
      m_timsrc_gdl_time[1] = ecl;
      m_timsrc_gdl_time[0] = cdc;
    }

    /** get timestamp when GDL receives timing sources*/
    unsigned getTimsrcGdlTime(int i) {return m_timsrc_gdl_time[i];}

    /** set t0 timestamp values in clk1nsec */
    void setT0(int top, int ecl, int cdc)
    {
      m_t0[2] = top;
      m_t0[1] = ecl;
      m_t0[0] = cdc;
    }

    /** get t0 timestamp values in clk1nsec*/
    unsigned getT0(int i) {return m_t0[i];}

    /** get timestamp value when GDL receives common L1 */
    unsigned getComL1Time(void) {return m_comL1_time;}

    /** set timestamp value when GDL receives common L1 */
    void setComL1Time(int comL1) {m_comL1_time = comL1;}

    /** get timestamp value when GDL generates L1 */
    unsigned getGdlL1Time(void) {return m_gdlL1_time;}

    /** set timestamp value when GDL generates L1 */
    void setGdlL1Time(int gdlL1) {m_gdlL1_time = gdlL1;}

    /**set the timType */
    void setTimType(TRGSummary::ETimingType timType) {m_timType = timType;}

    /*! get timing source information
     * @return     timing source type
     */
    TRGSummary::ETimingType getTimType() const
    {
      return m_timType;
    }

  private:

    /** timestamp when GDL receives timing src, 2:top,1:ecl,0:cdc */
    unsigned int m_timsrc_gdl_time[3] = {0};

    /** estimated t0 timestamp of 2:top,1:ecl,0:cdc */
    unsigned int m_t0[3] = {0};

    /** timestamp when GDL receives common L1 */
    unsigned int m_comL1_time = 0;

    /** timestamp when GDL generates L1 */
    unsigned int m_gdlL1_time = 0;

    /** types of trigger timing source defined in b2tt firmware */
    TRGSummary::ETimingType m_timType = TRGSummary::TTYP_NONE;

  };

} // end namespace Belle2

//#endif
