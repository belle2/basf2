"""
This file contains variables globally used in the analysis.
Variales are grouped in logical classes (python dictionaries), and typicaly that's classes who are called.
Each group is organised as follows:
Group_Of_Variables = {
    'Variable1': {
        'tuple_name':'<Name of the variable used in nTuple>',
        'lLim': <lower limit of the variable. Will be used for drawing>,
        'uLim': <upper limit of the variable. Will be used for drawing>,
        'xaxis':"<root-TeX-style name of the variable used to name axes on root plots>",
        'MClLim': <lower limit of the variable at gen-level. Will be used for drawing. This field is optional>,
        'MCuLim': <upper limit of the variable at gen-level. Will be used for drawing. This field is optional>,
        'units':"<TeX-style units of the variabel. This field is optional>",
        'logY': <False|True. If variable should be drawn with LogY. This field is optional>,
        'tex_name':"<TeX-style name of the variable used in formatting the tables. This field is optional>"
        },
    'Variable2':{...}
    ...
}
If you modify original ntuples, please make changes of tuple names here accordingly.
"""

#: Fit variables
fit_variables = {
    'Mbc':
        {'tuple_name': 'B_Mbc_corr',
         'lLim': 5.255,
         'MClLim': 5.265,
         'uLim': 5.29,
         'MCuLim': 5.29,
         'xaxis': "M_{bc}, GeV/c^{2}",
         'units': "GeV/c^{2}",
         'logY': False,
         'tex_name': "$\\text{M}_{\\text{bc}}$"},
    'deltaE':
        {'tuple_name': 'B_deltaE_corr',
         'lLim': -0.1,
         'MClLim': -0.015,
         'uLim': 0.1,
         'MCuLim': 0.015,
         'xaxis': "#Delta E, GeV",
         'units': "GeV",
         'logY': False,
         'tex_name': "$\Delta$E"},
    'Mrho':
        {'tuple_name': 'B_MR',
         'lLim': 0.5,
         'uLim': 1.05,
         'xaxis': "M_{#pi#pi}, GeV/c^{2}",
         'units': "GeV/c^{2}",
         'logY': False,
         'tex_name': "$\\text{M}_{\pi\pi}$"},
    'MKST':
        {'tuple_name': 'B_MK',
         'lLim': 0.792,
         'uLim': 0.992,
         'xaxis': "M_{K#pi}, GeV/c^{2}",
         'units': "GeV/c^{2}",
         'logY': False,
         'tex_name': "$\\text{M}_{\\text{K}\\pi}$"},
    'rhohel':
        {'tuple_name': 'B_helR',
         'lLim': -1,
         'uLim': 1,
         'xaxis': "cos(#theta_{#pi#pi})",
         'units': " ",
         'logY': False,
         'tex_name': "$\\cos(\\theta_{\\pi\\pi})$"},
    'Ksthel':
        {'tuple_name': 'B_helK',
         'lLim': -1,
         'uLim': 1,
         'xaxis': "cos(#theta_{K#pi})",
         'units': " ",
         'logY': False,
         'tex_name': "$\\cos(\\theta_{\\text{K}\\pi})$"}
}
