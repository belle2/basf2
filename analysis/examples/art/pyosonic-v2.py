from pyo import *
import time


class Effect(object):

    """ Parameters
        Speed (double time, half time) can be used for special events
        Volume could be used to indicate total energy of the event
    """

    def __init__(self, **modulation):
        self.speed = SigTo(0.5, init=0.5, time=1.0, mul=0.1, add=0.08)
        self.volume = SigTo(0.5, init=0.5, time=1.0)
        self.modulation = {key: SigTo(0.5, init=0.5, **value) for key, value in modulation.items()}


class Drums(Effect):

    """ Parameters
        Depth is inverse to the total energy in the ECL and KLM
    """

    def __init__(self):
        super(Drums, self).__init__(depth=dict())
        self.trg = Beat(time=self.speed, taps=16, w1=[90, 80], w2=50, w3=30, poly=1).play()
        envelope = CosTable([(0, 0), (200, 1), (400, 0.6), (600, .2), (6000, 0)])
        self.env = TrigEnv(self.trg, table=envelope, dur=self.trg['dur'], mul=self.trg['amp'])
        self.lfd = Sine([.4, .3], mul=.2, add=self.modulation['depth'])
        self.saw = SuperSaw(freq=[49, 50], detune=self.lfd, bal=self.modulation['depth'], mul=0.9 * self.volume * self.env).out()


class Instrument(Effect):

    """ Parameters
        Sharpness is the total PID information, so how well defined the particles are
    """

    def __init__(self):
        super(Instrument, self).__init__(sharpness=dict())
        self.trg = Beat(time=self.speed, taps=16, w1=[90, 80], w2=50, w3=30, poly=1).play()
        self.mid = TrigXnoiseMidi(self.trg, dist=12, mrange=(60, 72))
        self.fhz = Snap(self.mid, choice=[0, 2, 3, 5, 7, 8, 10], scale=1)
        envelope = CosTable([(0, 0), (400, 1), (800, 0.6), (5000, .5), (8100, 0)])
        self.env = TrigEnv(self.trg, table=envelope, dur=self.trg['dur'], mul=self.trg['amp'])
        self.lfo = LFO(freq=self.fhz, mul=0.7 * self.volume * self.env, sharp=self.modulation['sharpness'], type=7).out()


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
        self.rat = Snap(self.modulation['ratio'], choice=range(2, 9), scale=0)
        self.osc = SumOsc(
            freq=self.fhz,
            ratio=[
                1.0 /
                self.rat,
                self.rat],
            index=self.env,
            mul=[
                0.2 *
                self.volume,
                0.02 *
                self.volume]).out()


class Voices(Effect):

    """ Parameters
        Purity corresponds to total number of tracks in the event
    """

    def __init__(self):
        super(Voices, self).__init__(purity=dict(mul=40, add=10))
        self.trg = Beat(time=4 * self.speed, taps=16, w1=80, w2=50, w3=0, poly=1).play()
        self.mid = TrigXnoiseMidi(self.trg, dist=12, mrange=(84, 96))
        # Use only moll-pentatonik here
        self.fhz = Snap(self.mid, choice=[0, 3, 5, 7, 10], scale=1)
        self.sig = SigTo(self.fhz, time=2 * self.speed)
        self.lml = Lorenz(pitch=.003, stereo=True, mul=0.5, add=.8)
        self.lat = Lorenz(pitch=[.4, .38], mul=self.lml)
        self.res = Resonx(self.lat, freq=self.sig, q=self.modulation['purity'], mul=self.volume).out()
        self.hrm = Harmonizer(self.res, transpo=-5, winsize=0.05).out(1)


if __name__ == '__main__':
    s = Server()
    s.boot()
    s.start()

    drums = Drums()
    voice = Voices()
    instr = Instrument()
    bells = Bells()

    time.sleep(50)

    s.stop()
