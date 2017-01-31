import signal
import os
import sys

got_signal = False
print('blocked signals:', signal.pthread_sigmask(signal.SIG_BLOCK, []))


def sigquithandler(sig, frame):
    global got_signal
    print('handler called for ', sig)
    got_signal = True

# TODO investigating test failure on bamboo. (fails to die on SIGQUIT in single process.)
signals = [signal.SIGUSR1, signal.SIGUSR2, signal.SIGQUIT]
for sig in signals:
    got_signal = False
    prevsig = signal.signal(sig, sigquithandler)
    print('handler installed, previous handler was: ', prevsig)
    pid = os.getpid()
    print("Killing %s in init (sig %d)" % (pid, sig))
    os.kill(pid, sig)

    if not got_signal:
        print("signal missed, abort!")
        sys.exit(1)

print('Everything fine.')
