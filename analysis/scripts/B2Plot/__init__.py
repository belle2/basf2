"""
Thomas Keck 2013-2015
"""

# @cond internal
# only public interfaces, to make package doc nicer. Also the only things imported by 'from B2Plot import *'
__all__ = ['Plotter', 'plot_write', 'roc_purity_plot', 'roc_rejection_plot', 'distribution_plot',
           'difference_plot', 'correlation_plot', 'overtraining_plot', 'diagonal_plot']
# @endcond

from B2Plot.b2plot import Plotter
from B2Plot.b2plot import write as plot_write
from B2Plot.b2plot import roc_purity_plot, roc_rejection_plot, distribution_plot, \
    difference_plot, correlation_plot, overtraining_plot, diagonal_plot
