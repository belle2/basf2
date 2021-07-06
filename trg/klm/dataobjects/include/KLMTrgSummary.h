/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef KLMTrgSummary_H
#define KLMTrgSummary_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {
  //! Store KLM TRG track information as a ROOT object
  class KLMTrgSummary : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    KLMTrgSummary() = default;



    //! Destructor
    virtual ~KLMTrgSummary() { }

    // accessors



    void setBKLM_n_trg_sectors(int n_trg)
    {
      m_bklm_n_trg_sectors = n_trg;
    }


    void setEKLM_n_trg_sectors(int n_trg)
    {
      m_eklm_n_trg_sectors = n_trg;
    }
    void setBKLM_back_to_back_flag(int n_trg)
    {
      m_bklm_back_to_back_flag = n_trg;

    }
    void setEKLM_back_to_back_flag(int n_trg)
    {
      m_eklm_back_to_back_flag = n_trg;

    }


    void setSector_mask_Backward_Barrel(int mask)
    {
      Sector_mask_Backward_Barrel = mask;
    }

    void setSector_mask_Forward_Barrel(int mask)
    {
      Sector_mask_Forward_Barrel = mask;

    }
    void setSector_mask_Backward_Endcap(int mask)
    {
      Sector_mask_Backward_Endcap = mask;

    }
    void setSector_mask_Forward_Endcap(int mask)
    {
      Sector_mask_Forward_Endcap = mask;
    }




    int getBKLM_n_trg_sectors() const
    {
      return m_bklm_n_trg_sectors;
    }


    int  getEKLM_n_trg_sectors() const
    {
      return m_eklm_n_trg_sectors;
    }
    int getBKLM_back_to_back_flag() const
    {
      return m_bklm_back_to_back_flag ;

    }
    int getEKLM_back_to_back_flag() const
    {
      return m_eklm_back_to_back_flag ;

    }


    int  getSector_mask_Backward_Barrel() const
    {
      return Sector_mask_Backward_Barrel ;
    }

    int getSector_mask_Forward_Barrel() const
    {
      return Sector_mask_Forward_Barrel ;
    }

    int getSector_mask_Backward_Endcap() const
    {
      return Sector_mask_Backward_Endcap;
    }
    int getSector_mask_Forward_Endcap() const
    {
      return Sector_mask_Forward_Endcap ;
    }



    int  getSector_mask_OR_Backward_Barrel() const
    {
      return Sector_mask_OR_Backward_Barrel ;
    }

    int getSector_mask_OR_Forward_Barrel() const
    {
      return Sector_mask_OR_Forward_Barrel ;
    }

    int getSector_mask_OR_Backward_Endcap() const
    {
      return Sector_mask_OR_Backward_Endcap;
    }
    int getSector_mask_OR_Forward_Endcap() const
    {
      return Sector_mask_OR_Forward_Endcap ;
    }





    void setSector_mask_OR_Backward_Barrel(int mask)
    {
      Sector_mask_OR_Backward_Barrel = mask;
    }

    void setSector_mask_OR_Forward_Barrel(int mask)
    {
      Sector_mask_OR_Forward_Barrel = mask;

    }
    void setSector_mask_OR_Backward_Endcap(int mask)
    {
      Sector_mask_OR_Backward_Endcap = mask;

    }
    void setSector_mask_OR_Forward_Endcap(int mask)
    {
      Sector_mask_OR_Forward_Endcap = mask;
    }



  private:

    int  Sector_mask_Backward_Barrel = 0;
    int  Sector_mask_Forward_Barrel = 0;
    int  Sector_mask_Backward_Endcap = 0;
    int  Sector_mask_Forward_Endcap = 0;

    int  Sector_mask_OR_Backward_Barrel = 0;
    int  Sector_mask_OR_Forward_Barrel = 0;
    int  Sector_mask_OR_Backward_Endcap = 0;
    int  Sector_mask_OR_Forward_Endcap = 0;



    int  m_bklm_n_trg_sectors = 0;
    int  m_eklm_n_trg_sectors = 0 ;
    int  m_bklm_back_to_back_flag = 0 ;
    int  m_eklm_back_to_back_flag = 0;



    ClassDef(KLMTrgSummary, 5);
  };
} // end of namespace Belle2

#endif //KLMTrgSummary_H