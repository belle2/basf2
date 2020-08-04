#!/usr/bin/env python3

"""
Create sphinx documentation and make sure that there are no warnings.
However, a few warnings can not easily be omitted and are therefore accepted for now.
"""

import re
from b2test_utils import check_error_free

#: light build
light_build = False
try:
    import generators
except ModuleNotFoundError:
    light_build = True

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
    #: ignore online_book
    ignoreonlinebook = 'online_book'
    #: ignore further warnings in light builds
    #: ignore add_mdst_output of reconstruction package
    ignoreaddmdstoutput = 'add_mdst_output'
    #: ignore add_simulation of simulation package
    ignoreaddsimulation = 'add_simulation'
    #: ignore add_reconstruction of reconstruction package
    ignoreaddreconstruction = 'add_reconstruction'
    #: ignore missing include directives
    ignoreincludeproblem = 'Problems with "include" directive path'
    if light_build:
        check_error_free("b2code-sphinx-warnings", "sphinx", None,
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
                         re.findall(ignoreaddmdstoutput, x) or
                         re.findall(ignoreaddsimulation, x) or
                         re.findall(ignoreaddreconstruction, x) or
                         re.findall(ignoreincludeproblem, x) or
                         re.findall(ignoreonlinebook, x) or
                         re.findall(ignoregeometry, x),
                         ['--light']
                         )
    else:
        check_error_free("b2code-sphinx-warnings", "sphinx", None,
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
                         re.findall(ignoreonlinebook, x) or
                         re.findall(ignoregeometry, x)
                         )
