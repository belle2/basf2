##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
###########################################################################

# @cond internal
# The only things imported by 'from grafei import *'
__all__ = ["GraFEIModule"]
# @endcond

from grafei.modules.GraFEIModule import GraFEIModule

print(r"""
                                                  ____  ____    _      ____  ____  _
                                                  |  _  |__|   /_\     |___  |___  |
                                                  |__|  |  \  /   \    |     |___  |

                                o
                               / \
                              /   \                             x-----x                        ＿      ＿
                             /     \            |-----\         |\   /|        |-----\        |0 1 3 3 5|
                            o       \           |----- \        | \ / |        |----- \       |1 0 3 3 5|
                           / \       \          |----- /        |  x  |        |----- /       |3 3 0 1 5|
                          /   \       \         |-----/         | / \ |        |-----/        |3 3 1 0 5|
                         /     \       \                        |/   \|                       |5 5 5 5 0|
                        o       o       \                       x-----x                        ￣      ￣
                       / \     / \       \
                      x   x   x   x       x

      Authors: Jacopo Cerasoli, Giulio Dujany, Lucas Martel, Corentin Santos. 2022 - 2024
      Model description: https://indico.cern.ch/event/1106990/papers/4996235/files/12252-ACAT_2022_proceedings.pdf
      Based on the work of Kahn et al: https://iopscience.iop.org/article/10.1088/2632-2153/ac8de0
      Please consider citing both articles.
      Code adapted from https://github.com/Helmholtz-AI-Energy/BaumBauen
      """)
