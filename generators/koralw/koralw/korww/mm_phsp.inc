      PARAMETER (mm_nrchan=65)
! it was mm_monit
      COMMON /c_phspm/ 
     $                 prob_mm(mm_nrchan),
     $                 faki_mon(mm_nrchan+3000),
     $                 ikan_mon
      SAVE /c_phspm/
