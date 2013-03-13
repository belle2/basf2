*///////////////////////////////////////////////////////////////////////////////////////
*//                                                                                   //
*//          Pseudoclass FoamB                                                        //
*//                                                                                   //
*///////////////////////////////////////////////////////////////////////////////////////
*
*
      INTEGER        m_nBinMax
      PARAMETER  (   m_nBinMax =256 )
      INTEGER        m_NdiMax
      PARAMETER  (   m_NdiMax  = 5 )    ! maximum dimension for siplices
      INTEGER        m_KdiMax
      PARAMETER  (   m_KdiMax  = 15 )   ! maximum dimension for hypercubics
      INTEGER        m_DimMax
      PARAMETER  (   m_DimMax  = m_NdiMax+ m_KdiMax )   !  maximum total dimension
      INTEGER        m_nBufMax
      PARAMETER  (   m_nBufMax = 10000 )  ! maximum buffer length for all cells
      INTEGER        m_vMax
      PARAMETER  (   m_vMax    = 10000  ) ! maximum number of vertices
      INTEGER        m_cMax
      PARAMETER  (   m_cMax    = 10000 )  ! maximum number of (active) Cells
      INTEGER        m_sMax
      PARAMETER  (   m_sMax    = 5 )     ! statistics of the weight
      INTEGER        m_NpairMax
      PARAMETER  (   m_NpairMax  = m_NdiMax*(m_NdiMax+1)/2 ) ! no. of pairs of vertices
      INTEGER        m_NdiviMax
      PARAMETER  (   m_NdiviMax  = m_NpairMax+m_KdiMax ) ! Total no. of division samplings
*
      INTEGER            m_CeStat,   m_CePare,   m_CeDau1,  m_CeDau2,  m_CeSamp, m_CeVert, m_CeBest !
      DOUBLE PRECISION   m_CeIntg,   m_CeSum,    m_CeDriv,  m_CePrim,  m_CePrCu, m_CeXave, m_CeVolu !
      INTEGER            m_LastCe,   m_LastAc,   m_nBuf,    m_nBin,    m_LastVe !
      INTEGER            m_ActC
      DOUBLE PRECISION   m_VerX,     m_VolTot   !
      INTEGER            m_Ndim,     m_Kdim,     m_Dimen
      INTEGER            m_Chat,     m_Out,      m_nSampl,  m_Ncalls !
      INTEGER            m_OptPeek,  m_OptDrive, m_OptEdge, m_OptOrd,  m_EvPerBin, m_OptRanIni, m_OptRanLux!
      DOUBLE PRECISION   m_Drive,    m_SumWt,    m_SumWt2,  m_NevGen,    m_WtMax,   m_WtMin !
      DOUBLE PRECISION   m_MCresult, m_MCerror,  m_MCwt,    m_MCvector !
      DOUBLE PRECISION   m_CeVer1,   m_CeVer2
      INTEGER            m_Ltx,      m_MagicInit
*
      COMMON /c_FoamB/   
     $ m_CeStat(m_nBufMax),           ! Cell member: status=0 inactive, =1 active
     $ m_CePare(m_nBufMax),           ! Cell member: parent cell pointer
     $ m_CeDau1(m_nBufMax),           ! Cell member: daughter1 cell pointer
     $ m_CeDau2(m_nBufMax),           ! Cell member: daughter2 cell pointer
     $ m_CeSamp(m_nBufMax),           ! Cell member: No of MC events in exploration
     $ m_CeVert(m_nBufMax,m_NdiMax+1),! Cell member: simplex vertex pointers
     $ m_CeVer1(m_nBufMax,m_KdiMax),  ! Cell member: hypercubic vertex position
     $ m_CeVer2(m_nBufMax,m_KdiMax),  ! Cell member: hypercubic vertex size
     $ m_CeIntg(m_nBufMax),           ! Cell member: integral estimator
     $ m_CeDriv(m_nBufMax),           ! Cell member: Drive integral estimate, from build-up
     $ m_CePrim(m_nBufMax),           ! Cell member: Primary integral estimate, MC generation
     $ m_CePrCu(0:m_nBufMax),         ! Cell member: Cumulative Primary 
     $ m_CeVolu(m_nBufMax),           ! Cell member: Cartesian volume
     $ m_CeXave(m_nBufMax),           ! Cell member: Average best X
     $ m_CeBest(m_nBufMax),           ! Cell member: Best pair of vertices, pointer
     $ m_CeSum( m_nBufMax,m_sMax),    ! Cell member: weight summaries
     $ m_VerX(  m_vMax, m_NdiMax), ! List of all VERTEX positions
     $ m_ActC(m_cMax),             ! List of all pointers to ACTIVE cells
     $ m_VolTot,                   ! Estimate of Volume total, without error
     $ m_Drive,             ! M.C. generation Drive value of integral
     $ m_SumWt,             ! M.C. generation sum of Wt
     $ m_SumWt2,            ! M.C. generation sum of Wt**2
     $ m_NevGen,            ! M.C. generation sum of 1d0
     $ m_WtMax,             ! M.C. generation maximum wt
     $ m_WtMin,             ! M.C. generation minimum wt
     $ m_MCresult,          ! M.C. generation Final value of INTEGRAL
     $ m_MCerror,           ! M.C. generation Final walue of ERROR
     $ m_MCwt,              ! M.C. generation current event weight
     $ m_MCvector(m_DimMax),! M.C. generated vector, dimension=m_NdiMax+ m_KdiMax
     $ m_Ndim,              ! dimension of the siplices
     $ m_Kdim,              ! dimension of the hypercubics
     $ m_Dimen,             ! total dimension of the problem =m_Ndim+m_Kdim
     $ m_nBuf,              ! Actual dynamic lenth of the buffer m_nBuf<m_nBufMax
     $ m_nBin,              ! No of bins in histogram for cell exploration/division
     $ m_LastVe,            ! Last vertex
     $ m_LastAc,            ! Last active cell
     $ m_LastCe,            ! Last cell in buffer 
     $ m_nSampl,            ! No. of sampling when dividing cell
     $ m_Ncalls,            ! No. of function calls, total
     $ m_OptPeek,           ! Flag for  random ceel choice: Peek =0,1 for maximum,
     $ m_OptDrive,          ! Flag for type of Drive =0,1,2 for TrueVol,Sigma,WtMax
     $ m_OptEdge,           ! Flag which decides whether vertices are included in the sampling
     $ m_OptOrd,            ! Flag =0 default, entire domain is hyp-cubic, =1 it is simplex
     $ m_EvPerBin,          ! Maximum effective eevents per bin, saves CPU, =0 inactive
     $ m_OptRanIni,         ! Flag =0 default, =1 r.n. generator not initialized in PianA
     $ m_OptRanLux,         ! Flag =-1,0,1,2,3,4 r.n. generator level
     $ m_Chat,              ! Flag for chat level in output, Chat=1 normal level
     $ m_Ltx,               ! Latex Output unit, for debug
     $ m_Out,               ! Output unit
     $ m_MagicInit          ! Magic cookie of initialization (global variable)
*
      SAVE /c_FoamB/
*//////////////////////////////////////////////////////////////////////////////////////
*//                                                                                  //
*//////////////////////////////////////////////////////////////////////////////////////
