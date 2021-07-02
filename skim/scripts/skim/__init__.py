#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Skim Package
------------

Core classes of skim package are provided as convenient imports directly from ``skim``.

To use a specific skim, import the skim from the appropriate working group submodule in
``skim.WGs``.

    from skim.WGs.systematics import RadEEList
"""

# Provide core classes of skim package
from skim.core import BaseSkim, CombinedSkim  # noqa
from skim.utils.misc import fancy_skim_header  # noqa
