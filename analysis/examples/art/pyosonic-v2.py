#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# Thomas Keck 2015
#
# This sonification module used pyo
# http://ajaxsoundstudio.com/software/pyo/
# You have to install pyo to use it ;-)

# Add system python libraries because pyo isn't available via pip
import sys
sys.path.append("/usr/lib/python2.7/dist-packages/")

from pyo import *

from basf2 import *
from ROOT import Belle2
import modularAnalysis
from variables import variables
import time


class Effect(object):

    """ Parameters
        Speed (double time, half time) can be used for special events
        Volume could be used to indicate total energy of the event
        Modulation See subclasses
    """

    def __init__(self, **modulation):
        self.speed = SigTo(0.5, init=0.5, time=1.0, mul=0.1, add=0.08)
        self.volume = SigTo(0.5, init=0.5, time=1.0)
        self.modulation = {key: SigTo(0.5, init=0.5, **value) for key, value in list(modulation.items())}


class Drums(Effect):

    """ Parameters
        Depth
    """

    def __init__(self):
        super(Drums, self).__init__(depth=dict())
        self.trg = Beat(time=self.speed, taps=16, w1=[90, 80], w2=50, w3=30, poly=1).play()
        envelope = CosTable([(0, 0), (200, 1), (400, 0.6), (600, .2), (6000, 0)])
        self.env = TrigEnv(self.trg, table=envelope, dur=self.trg['dur'], mul=self.trg['amp'])
        self.lfd = Sine([.4, .3], mul=.2, add=self.modulation['depth'])
        self.saw = SuperSaw(freq=[49, 50], detune=self.lfd, bal=self.modulation['depth'], mul=self.volume * self.env).out()


class Instrument(Effect):

    """ Parameters
        Sharpness is the total PID information, so how well defined the particles are
    """

    def __init__(self):
        super(Instrument, self).__init__(sharpness=dict(mul=0.1, time=2.0))
        self.trg = Beat(time=self.speed, taps=16, w1=[90, 80], w2=50, w3=30, poly=1).play()
        self.mid = TrigXnoiseMidi(self.trg, dist=12, mrange=(60, 72))
        self.fhz = Snap(self.mid, choice=[0, 2, 3, 5, 7, 8, 10], scale=1)
        envelope = CosTable([(0, 0), (400, 1), (800, 0.6), (5000, .5), (8100, 0)])
        self.env = TrigEnv(self.trg, table=envelope, dur=self.trg['dur'], mul=self.trg['amp'])
        self.lfo = LFO(freq=self.fhz, mul=0.5 * self.volume * self.env, sharp=self.modulation['sharpness'], type=0).out()


class Bells(Effect):

    """ Parameters
        Ratio
    """

    def __init__(self):
        super(Bells, self).__init__(ratio=dict(mul=5, add=2))
        self.trg = Beat(time=self.speed, taps=16, w1=[90, 80], w2=50, w3=30, poly=1).play()
        self.mid = TrigXnoiseMidi(self.trg, dist=12, mrange=(60, 72))
        self.fhz = Snap(self.mid, choice=[0, 2, 3, 5, 7, 8, 10], scale=1)
        envelope = LinTable([(0, .3), (20, .85), (300, .7), (1000, .5), (8191, .3)])
        self.env = TrigEnv(self.trg, table=envelope, dur=self.trg['dur'])
        self.rat = Snap(self.modulation['ratio'], choice=list(range(2, 9)), scale=0)
        self.osc = SumOsc(
            freq=self.fhz,
            ratio=[
                1.0 /
                self.rat,
                self.rat],
            index=self.env,
            mul=[
                0.5 *
                self.volume,
                0.05 *
                self.volume]).out()


class Voices(Effect):

    """ Parameters
        Purity corresponds to total number of tracks in the event
    """

    def __init__(self):
        super(Voices, self).__init__(purity=dict(mul=30, add=5))
        self.trg = Beat(time=2 * self.speed, taps=16, w1=80, w2=50, w3=0, poly=1).play()
        self.mid = TrigXnoiseMidi(self.trg, dist=12, mrange=(84, 96))
        # Use only moll-pentatonik here
        self.fhz = Snap(self.mid, choice=[0, 3, 5, 7, 10], scale=1)
        self.sig = SigTo(self.fhz, time=self.speed)
        self.lml = Lorenz(pitch=.003, stereo=True, mul=0.5, add=.8)
        self.lat = Lorenz(pitch=[.4, .38], mul=self.lml)
        self.res = Resonx(self.lat,
                          freq=self.sig,
                          q=self.modulation['purity'],
                          mul=0.3 * (0.5 + self.modulation['purity']) * self.volume).out()
        self.hrm = Harmonizer(self.res, transpo=-5, winsize=0.05).out(1)


def normalise(value, min_v, max_v):
    return max(min(float(value - min_v) / float(max_v - min_v), 1.0), 0.0)


class Sonic(Module):

    def __init__(self):
        super(Sonic, self).__init__()
        self.drums = Drums()
        self.drums.volume.setValue(0.8)
        self.voice = Voices()
        self.instr = Instrument()
        self.bells = Bells()

    def event(self):
        plist = Belle2.PyStoreObj('e+')
        ntracks = plist.obj().getListSize()
        self.voice.modulation['purity'].setValue(normalise(ntracks, 2, 20))
        print("#Tracks", ntracks, normalise(ntracks, 2, 20))

        energy = variables.evaluate('ECLEnergy', 0) + variables.evaluate('KLMEnergy', 0)
        self.instr.volume.setValue(normalise(energy, 0, 20))
        self.bells.volume.setValue(1 - normalise(energy, 0, 20))
        print("Energy", energy, normalise(energy, 0, 20))

        trackprob = 0
        pidprob = 0

        for i in range(ntracks):
            p = plist.obj().getParticle(i)
            eid = variables.evaluate('electronID', p)
            kid = variables.evaluate('kaonID', p)
            prid = variables.evaluate('protonID', p)
            muid = variables.evaluate('muonID', p)
            piid = variables.evaluate('pionID', p)
            pidprob += max([eid, kid, prid, muid])
            trackprob += variables.evaluate('chiProb', p)
        self.instr.modulation['sharpness'].setValue(normalise(pidprob, 0, ntracks))
        print("PIDProb", pidprob, normalise(pidprob, 0, ntracks))

        self.drums.modulation['depth'].setValue(normalise(trackprob, 0, ntracks))
        print("TrackProb", trackprob, normalise(trackprob, 0, ntracks))

        gammaprob = 0
        plist = Belle2.PyStoreObj('gamma')
        ngammas = plist.obj().getListSize()
        for i in range(ngammas):
            p = plist.obj().getParticle(i)
            gammaprob += variables.evaluate('goodGamma', p)

        self.bells.modulation['ratio'].setValue(normalise(gammaprob, 0, ngammas))
        print("GammaProb", gammaprob, normalise(gammaprob, 0, ngammas))

        time.sleep(5)


s = Server()
s.recordOptions(dur=10, filename='test.ogg', fileformat=7)
s.boot()
# Start server
s.start()
s.recstart()

path = create_path()
path.add_module('RootInput')
path.add_module('Gearbox')
path.add_module('ProgressBar')
modularAnalysis.fillParticleList('e+', '', path=path)
modularAnalysis.fillParticleList('gamma', '', path=path)
path.add_module(Sonic())
process(path)

s.recstop()
s.stop()
