#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from ._version import get_versions
from alignment.fancystuff.tools import draw_fancy_correlation_matrix, draw_flat_corr_matrix, draw_flat_correlation  # noqa

__version__ = get_versions()['version']
del get_versions
