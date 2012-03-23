#!/bin/tcsh -f



# User specific aliases and functions

######################################################################
# post-system rc
if ( -f $BHOME/local/etc/cshrc_general ) then
     source $BHOME/local/etc/cshrc_general
endif
#source ~/.bellelib_cshrc.20070528.csh
#####################################################################
# MY Project Lib

#setenv PHI_LIB "/bwf/g39home/jhkim/Analysis/"
#####################################################################

make clean
make 



