#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from rawdata_compatibility_base import test_raw

if __name__ == "__main__":
    # use the None as GT so the default global tag will be used for Monte Carlo generated files
    test_raw("phase3", "mc", None)
