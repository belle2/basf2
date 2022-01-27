##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


# Script to run an evalution of the trained background remover expert. Or the root weigthfile defining 
# its behaviour. If latex is installed, its created the file output.pdf with many insigthful plots. 
#
# Usage:
#
# bash evaluate.sh 



basf2_mva_evaluate.py  -id weightfile.root \
                    -train train.root \
                    -data test.root \
                    -c \
                    --outputfile output.pdf
                    
                    
                    
