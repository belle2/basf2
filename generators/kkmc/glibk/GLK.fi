*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                       Pseudo-Class  GLK                                  //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////

*//////////////////////////////////////////////////////////////////////////////
*// =======================================================================  //
*// ==========================  _GLK_  ====================================  //
*// ========== General Library of histogramming/ploting utilities =========  //
*// ========== It is similar but not identical to HBOOK and HPLOT =========  //
*// =======================================================================  //
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                      Version:    1.31                                    //
*//              Last correction:    May 2000                                //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////

      INTEGER    m_version         ! this is the actual version number
      PARAMETER( m_version = 140)
*-----------------------------------------------------------------------------
      INTEGER m_idmax              ! maximum number of bins
      PARAMETER( m_idmax=400)
* Parameters m_buf1 and m_buf2 are lenths of buffors/headers in the histogram
* m_buf1 is for 1-dim histogram and m_buf2 for 2-dim histo
* m_buf2 is still not very consequently implemented 
* so m_buf1=m_buf2 is still required
      INTEGER    m_buf1,    m_buf2
      PARAMETER( m_buf1=24, m_buf2=24)
      INTEGER    m_MaxNb
      PARAMETER( m_MaxNb = 50000)   ! maximum numbers of bins
      INTEGER    m_MaxNlab
      PARAMETER( m_MaxNlab = 100)   ! maximum numbers labels in table 1-st col.
*-----------------------------------------------------------------------------
      INTEGER       m_LenmB
      PARAMETER    (m_LenmB = 100000)
      DOUBLE PRECISION   m_b
      INTEGER      m_out, m_huni, m_length, m_LenInd, m_index
      CHARACTER*80 m_titlc
      INTEGER      m_lint, m_ltx, m_KeyTit
      CHARACTER*80 m_titch         ! title of plot, several lines
      INTEGER      m_titlen        ! number of lines
      PARAMETER(   m_titlen =50)
      CHARACTER*1  m_BS            ! backslash character
      INTEGER      m_tline         ! variable controling style of line or mark
      CHARACTER*80 m_Color         ! string defining color
      INTEGER      m_KeyCol        ! control of color use, reset internaly!
      INTEGER      m_KeyTbr        ! control of table range, reset after use!
      INTEGER      m_TabRan        ! Table range for ploting
      INTEGER      m_KeyTbl        ! Tag for Table first column label
      CHARACTER*32 m_TabLab        ! label in 1-st column of table
*-----------------------------------------------------------------------------
      COMMON  /c_GLK/ 
     $   m_b(m_LenmB),             ! storage for all histograms
     $   m_out,                    ! general output for printouts
     $   m_huni,                   ! disk unit number for histos
     $   m_length,                 ! act. length of m_b   (less than m_LenmB )
     $   m_LenInd,                 ! act. length of index, UNDER DEVELOPEMENT!
     $   m_index(m_idmax,3),       ! index of histos
     $   m_titlc(m_idmax),         ! titles of histos
     $   m_tline,                  ! general output for printouts
     $   m_lint,                   ! argument of gplint, sets various styles
     $   m_ltx,                    ! output unit for latex
     $   m_titch(m_titlen),        ! title of plot, several lines
     $   m_Color,                  ! string defining color
     $   m_KeyTit,                 ! status key of plot Title
     $   m_KeyCol,                 ! status key of Color
     $   m_KeyTbr,                 ! status key of Table range for ploting
     $   m_TabRan(3),              ! Table range for ploting
     $   m_KeyTbl,                 ! status for Table first column label
     $   m_TabLab(m_MaxNlab),      ! Label in 1-st column of table
     $   m_BS                      ! backslash for LaTex (machine dependent!)

*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                      End of CLASS  GLG                                   //
*//////////////////////////////////////////////////////////////////////////////
