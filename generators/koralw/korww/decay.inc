      PARAMETER(max_e_bin=1000,max_label=202)
      COMMON /c_decay/ prob_chan(max_e_bin,max_label),       !plain pr.
     $                 prob_chan_cumul(max_e_bin,max_label), !cumul. pr
     $                 prob_e_total(max_e_bin),              !total pr.
     $                 emin,emax,      ! range of svar in prob. matrices
     $                 umask_lbl(max_label)  ! extrnal mask on decay ch
      SAVE /c_decay/
