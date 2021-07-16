import ROOT
from ROOT import Belle2
from basf2 import Module


class NeutralHadron4MomentumCalculator(Module):

    def __init__(self, particleList, path=None):
        super().__init__()
        self.particleList = particleList
        self.path = path
        self.path.add_module(self)

        self.neutralDirection = ROOT.TVector3()
        self.neutral3Momentum = ROOT.TVector3()
        self.neutral4Momentum = ROOT.TLorentzVector()
        self.toRemove = ROOT.std.vector('unsigned int')()

    def event(self):
        particleList = Belle2.PyStoreObj(self.particleList).obj()
        for particle in particleList:
            charged = particle.getDaughter(0)
            neutral = particle.getDaughter(1)
            self.neutralDirection = neutral.getECLCluster().getClusterPosition().Unit()
            a = charged.getMomentum() * self.neutralDirection
            b = (particle.getPDGMass()**2 - neutral.getMass()**2 - charged.get4Vector().Mag2()) / 2.
            c = charged.getEnergy()
            d = neutral.getMass()**2
            D = (a**2 - c**2) * d + b**2
            if D >= 0:
                neutralP = (-1. * a * b - c * D**(0.5)) / (a**2 - c**2)
                self.neutral3Momentum.SetMagThetaPhi(neutralP, self.neutralDirection.Theta(), self.neutralDirection.Phi())
                self.neutral4Momentum.SetVectM(self.neutral3Momentum, neutral.getMass())
                neutral.set4Vector(self.neutral4Momentum)
                particle.set4Vector(neutral.get4Vector() + charged.get4Vector())
            else:  # Remove this Particle from its ParticleList
                self.toRemove.push_back(particle.getArrayIndex())
        particleList.removeParticles(self.toRemove)
        self.toRemove.clear()
