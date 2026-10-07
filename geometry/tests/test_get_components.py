#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Test `geometry.get_components()` (experiment-number -> canonical detector
component list) and the mutual-exclusion checks in `geometry.check_components()`
that keep VTX from being combined with PXD/SVD, and the CDC alternative
components (STL/FTL/rCDC) from being combined with plain CDC.
'''

import subprocess
import sys

from geometry import get_components, check_components, is_detector_present


def expect_failure(code):
    """Run the given one-line python snippet in a subprocess and assert it fails.

    basf2.B2FATAL aborts the process outright, so it cannot be caught with a
    normal Python try/except inside the same process; running it in a
    subprocess and checking the exit code is the only reliable way to test it.
    """
    result = subprocess.run([sys.executable, '-c', code], capture_output=True, text=True)
    assert result.returncode != 0, f"Expected failure, but succeeded: {code!r}\n{result.stdout}{result.stderr}"


if __name__ == '__main__':
    # --- Standard Belle II (PXD/SVD), below the upgrade experiment range ---
    standard = get_components(0)
    assert 'PXD' in standard and 'SVD' in standard and 'VTX' not in standard
    check_components(standard)  # must not raise
    assert get_components(1999) == standard, "exp 1999 should still resolve to standard Belle II components"

    # --- VTX upgrade scenarios (currently registered: exp 2003-2006) ---
    for exp in (2003, 2004, 2005, 2006):
        components = get_components(exp)
        assert 'VTX' in components, f"exp {exp} should use VTX"
        assert 'PXD' not in components and 'SVD' not in components, f"exp {exp} should not use PXD/SVD"
        check_components(components)  # each registered set must be internally consistent
        assert is_detector_present('VTX', components)
        assert not is_detector_present('PXD', components)
        assert not is_detector_present('SVD', components)

    # get_components() must return a fresh list each time, not a shared/mutable reference
    a, b = get_components(0), get_components(0)
    assert a == b and a is not b

    # --- Unregistered experiment numbers must fail loudly, not silently pick the
    #     wrong detector configuration. This also covers the gap between the
    #     standard range (< 2000) and the upgrade ranges (2003-2006): exp 2000-2002
    #     and >= 2007 are not currently registered. ---
    for exp in (-1, 2000, 2001, 2002, 2007, 2023):
        expect_failure(f'from geometry import get_components; get_components({exp})')

    # --- check_components() must reject VTX combined with PXD/SVD ---
    expect_failure("from geometry import check_components; check_components(['VTX', 'SVD'])")
    expect_failure("from geometry import check_components; check_components(['VTX', 'PXD'])")

    # --- ... and must reject the CDC-alternative components combined with plain CDC ---
    expect_failure("from geometry import check_components; check_components(['STL', 'CDC'])")
    expect_failure("from geometry import check_components; check_components(['FTL', 'CDC'])")
    expect_failure("from geometry import check_components; check_components(['rCDC', 'CDC'])")

    # --- ... but PXD+SVD together (standard, no VTX), and the CDC-alternative
    #     components together (no plain CDC), must still be allowed ---
    check_components(['PXD', 'SVD', 'CDC'])
    check_components(['VTX', 'STL', 'FTL', 'rCDC'])

    # components=None ("use everything", the add_simulation()/add_reconstruction()
    # default) must remain untouched by any of this.
    check_components(None)

    print('All geometry.get_components() / check_components() checks passed.')
