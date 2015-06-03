from ._version import get_versions

__version__ = get_versions()['version']
del get_versions


from tools import draw_fancy_correlation_matrix, draw_flat_corr_matrix, draw_flat_correlation
