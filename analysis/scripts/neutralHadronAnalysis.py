
import ROOT
from ROOT import Belle2
from basf2 import Module


class NeutralHadron4MomentumCalculator(Module):
    '''Calculates 4-momentum of a neutral hadron in a given decay chain e.g. B0 -> J/Psi K_L0, or anti-B0 -> p+ K- anti-n0.

    The momenta of the neutral hadron and its mother will be updated accordingly, as in the case of vertex-fitting.
    '''

    def __init__(self, particleList, path=None):
        '''Constructor for NeutralHadron4MomentumCalculator

        Parameters:
            particleList: ParticleList to calculate the
            path: path to put the module in

        Variables:
            self.neutralDirection: Direction of the neutral particle
            self.neutral3Momentum: Momentum of the neutral particle
            self.neutral4Momentum: 4-momentum of the nuetral particle
            self.toRemove: A vector to store indices of Particle in the Particle List to remove
        '''
        super().__init__()
        self._particleList = particleList
        self._path = path
        self._path.add_module(self)

        self._neutralDirection = ROOT.TVector3()
        self._neutral3Momentum = ROOT.TVector3()
        self._neutral4Momentum = ROOT.TLorentzVector()
        self._toRemove = ROOT.std.vector('unsigned int')()

    def event(self):
        '''Function to execute in each event'''
        particleList = Belle2.PyStoreObj(self._particleList).obj()
        for particle in particleList:
            charged = particle.getDaughter(0)
            neutral = particle.getDaughter(1)
            self._neutralDirection = neutral.getECLCluster().getClusterPosition().Unit()
            a = charged.getMomentum() * self.neutralDirection
            b = (particle.getPDGMass()**2 - neutral.getMass()**2 - charged.get4Vector().Mag2()) / 2.
            c = charged.getEnergy()
            d = neutral.getMass()**2
            D = (a**2 - c**2) * d + b**2
            if D >= 0:
                neutralP = (-1. * a * b - c * D**(0.5)) / (a**2 - c**2)
                self._neutral3Momentum.SetMagThetaPhi(neutralP, self._neutralDirection.Theta(), self._neutralDirection.Phi())
                self._neutral4Momentum.SetVectM(self._neutral3Momentum, neutral.getMass())
                neutral.set4Vector(self._neutral4Momentum)
                particle.set4Vector(neutral.get4Vector() + charged.get4Vector())
            else:  # Remove this Particle from its ParticleList
                self._toRemove.push_back(particle.getArrayIndex())
        particleList.removeParticles(self._toRemove)
        self._toRemove.clear()
