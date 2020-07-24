#!/usr/bin/env python3

"""
Create sphinx documentation and make sure that there are no warnings.
However, a few warnings can not easily be omitted and are therefore accepted for now.
"""

import re
from b2test_utils import check_error_free

if __name__ == "__main__":
    #: ignore html_static_path
    ignorehtmlstaticpath = 'html_static_path'
    #: ignore unsupported theme option
    ignoreunsupportedthemeoption = 'unsupported theme option'
    #: ignore environment variables
    ignoreenvironmentvariables = 'envvar'
    #: ignore the warnings about class references not found in python
    ignorepythonclass = 'py:class reference target not found'
    #: ignore the warnings about duplicated labels in whatsnew part
    ignoreduplicatewhatsnewlabel = 'duplicate label whatsnew:'
    #: ignore duplicate labels in calibration package (NEEDS TO BE FIXED!)
    ignoreduplicatecalibrationlabel = 'duplicate label calibration'
    #: ignore framework description of role warnings (should be fixed)
    ignoreduplicatedescriptionofrole = 'duplicate description of role'
    #: ignore duplicate labels in mva package
    ignoreduplicatemvalabel = 'duplicate label mva'
    #: ignore warning from tools repository
    ignorechoosingarelease = 'Choosing a release'
    #: ignore warning about not finding Geometry module ?!
    ignoregeometry = 'b2:mod reference target not found: Geometry'
    #: ignore skim registry warning (NEEDS TO BE FIXED!)
    ignoreskimregistry = 'skim.registry.combined_skims'
    check_error_free("b2code-sphinx-warnings", "sphinx", "",
                     lambda x:
                     re.findall(ignorehtmlstaticpath, x) or
                     re.findall(ignoreunsupportedthemeoption, x) or
                     re.findall(ignoreenvironmentvariables, x) or
                     re.findall(ignorepythonclass, x) or
                     re.findall(ignoreduplicatewhatsnewlabel, x) or
                     re.findall(ignoreduplicatecalibrationlabel, x) or
                     re.findall(ignoreduplicatedescriptionofrole, x) or
                     re.findall(ignoreduplicatemvalabel, x) or
                     re.findall(ignorechoosingarelease, x) or
                     re.findall(ignoregeometry, x) or
                     re.findall(ignoreskimregistry, x)
                     )
