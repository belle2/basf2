# This file was automatically created by FeynRules 2.3.26
# Mathematica version: 11.1.0 for Mac OS X x86 (64-bit) (March 16, 2017)
# Date: Tue 23 May 2017 14:45:12


from object_library import all_lorentz, Lorentz

from function_library import complexconjugate, re, im, csc, sec, acsc, asec, cot
try:
    import form_factors as ForFac
except ImportError:
    pass


UUV1 = Lorentz(name='UUV1',
               spins=[-1, -1, 3],
               structure='P(3,2) + P(3,3)')

SSS1 = Lorentz(name='SSS1',
               spins=[1, 1, 1],
               structure='1')

FFS1 = Lorentz(name='FFS1',
               spins=[2, 2, 1],
               structure='ProjM(2,1) + ProjP(2,1)')

FFV1 = Lorentz(name='FFV1',
               spins=[2, 2, 3],
               structure='Gamma(3,2,1)')

FFV2 = Lorentz(name='FFV2',
               spins=[2, 2, 3],
               structure='Gamma(3,2,-1)*ProjM(-1,1)')

FFV3 = Lorentz(name='FFV3',
               spins=[2, 2, 3],
               structure='Gamma(3,2,-1)*ProjP(-1,1)')

VVS1 = Lorentz(name='VVS1',
               spins=[3, 3, 1],
               structure='Metric(1,2)')

VVV1 = Lorentz(
    name='VVV1',
    spins=[
        3,
        3,
        3],
    structure='P(3,1)*Metric(1,2) - P(3,2)*Metric(1,2) - P(2,1)*Metric(1,3) + P(2,3)*Metric(1,3) + P(1,2)*Metric(2,3) - P(1,3)*Metric(2,3)')  # noqa

SSSS1 = Lorentz(name='SSSS1',
                spins=[1, 1, 1, 1],
                structure='1')

VVSS1 = Lorentz(name='VVSS1',
                spins=[3, 3, 1, 1],
                structure='Metric(1,2)')

VVVV1 = Lorentz(name='VVVV1',
                spins=[3, 3, 3, 3],
                structure='Metric(1,4)*Metric(2,3) - Metric(1,3)*Metric(2,4)')

VVVV2 = Lorentz(name='VVVV2',
                spins=[3, 3, 3, 3],
                structure='Metric(1,4)*Metric(2,3) + Metric(1,3)*Metric(2,4) - 2*Metric(1,2)*Metric(3,4)')

VVVV3 = Lorentz(name='VVVV3',
                spins=[3, 3, 3, 3],
                structure='Metric(1,4)*Metric(2,3) - Metric(1,2)*Metric(3,4)')

VVVV4 = Lorentz(name='VVVV4',
                spins=[3, 3, 3, 3],
                structure='Metric(1,3)*Metric(2,4) - Metric(1,2)*Metric(3,4)')

VVVV5 = Lorentz(name='VVVV5',
                spins=[3, 3, 3, 3],
                structure='Metric(1,4)*Metric(2,3) - (Metric(1,3)*Metric(2,4))/2. - (Metric(1,2)*Metric(3,4))/2.')
