#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
<output>validationTestColorScheme.root</output>
<contact>Kilian Lieret, Kilian.Lieret@campus.lmu.de</contact>
</header>
"""

"""
To test the color scheme.
Set generate_reference = True to generate the reference files.
"""

from validationtest import add_properties
from ROOT import TFile, TH1F
import basf2


def generate_gaus(title, options, distort=0.):
    """ Generate trivial gaus filled histogram.
    Use the distort parameter (0<=distort<=1) for distortion to get histograms
    with failing comparisons.
    """
    # fix seed, because we need to see failing and passing comparisons

    assert(0 <= distort <= 1)

    basf2.set_random_seed(10)

    norm = 300

    name = title.lower().replace(" ", "_")

    gaus = TH1F(name, title, 100, -3, 3)

    gaus.FillRandom("gaus", int((1 - distort) * norm))
    # add a different histogram
    basf2.set_random_seed(100)
    distortion = TH1F(name, title, 100, -3, 3)
    distortion.FillRandom("gaus", int(distort * norm))
    gaus.Add(distortion)

    add_properties(gaus, options)
    gaus.Write()


tfile = TFile("validationTestColorScheme.root", "RECREATE")

mop = {
    'Description': "Test color scheme",
    'Check': "Check color",
    'Contact': "Kilian Lieret, Kilian.Lieret@campus.lmu.de",
    'MetaOptions': "pvalue-error=0.6"
}
mop_expert = mop.copy()
mop_expert['MetaOptions'] += ", expert"

generate_reference = False

if not generate_reference:
    # No reference files for the plots that have to be checked manually
    generate_gaus("manual", mop)
    generate_gaus("manual expert", mop_expert)

if generate_reference:
    # The reference will contain the normal (undistorted) histogram
    # always.
    generate_gaus("pass", mop)
    generate_gaus("pass expert", mop_expert)
    generate_gaus("warn", mop)
    generate_gaus("fail", mop)
    generate_gaus("warn expert", mop_expert)
    generate_gaus("fail expert", mop_expert)
else:
    generate_gaus("pass", mop, distort=0.1)
    generate_gaus("pass expert", mop_expert, distort=0.1)
    generate_gaus("warn", mop, distort=0.5)
    generate_gaus("fail", mop, distort=1)
    generate_gaus("warn expert", mop_expert, distort=0.5)
    generate_gaus("fail expert", mop_expert, distort=1)

tfile.Close()
