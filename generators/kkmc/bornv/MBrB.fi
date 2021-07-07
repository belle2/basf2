*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                Pseudo-CLASS  MBrB (Multi-BRrancher)                      //
*//                                                                          //
*//   Purpose: Manage general multi-branched Monte Carlo algorithm           //
*//                                                                          //
*//   MBrB is capable to:                                                    //
*//    -register arbitrary number of branches                                //
*//    -keep track of elaborate MC statistics for each branch                //
*//    -keep track of elaborate MC statistics for all branches               //
*//    -generate randomly branch index                                       //
*//    -prowide useful averages and counts at the end of the run             //
*//    -print out statistics at the ned of the run                           //
*//   It is using weight monitoring tools from GLK library                   //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
*
*  Class members:
*
      INTEGER    m_MaxBra
      PARAMETER( m_MaxBra =  200 )       ! maximum number of branches

      DOUBLE PRECISION   m_WMList,  m_XsList
      INTEGER m_KFList, m_idMBR, m_Ntot, m_out
      INTEGER m_KFlast, m_IDlast

      COMMON /c_MBrB/
     $  m_KFList(m_MaxBra),              ! list of KFcodes
     $  m_WMList(m_MaxBra),              ! enhancement factors
     $  m_XsList(m_MaxBra),              ! primary x-sections
     $  m_IDlast,                        ! KF generated last time
     $  m_KFlast,                        ! branch generated last time
     $  m_idMBR,                         ! pointer for histograms
     $  m_Ntot,                          ! actual number of branches 
     $  m_out                            ! output unit number

*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                      End of CLASS  MBrB                                  //
*//////////////////////////////////////////////////////////////////////////////
