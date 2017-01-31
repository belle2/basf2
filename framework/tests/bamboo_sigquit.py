import signal
import os
import sys

pid = os.getpid()
ret = os.system('strace -o /dev/null true')
if ret != 0:
    print('Process is being traced!')
    sys.exit(1)

got_signal = False


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
    print("Killing %s in init (sig %d)" % (pid, sig))
    os.kill(pid, sig)

    if not got_signal:
        print("signal missed, abort!")
        sys.exit(1)

print('Everything fine.')
