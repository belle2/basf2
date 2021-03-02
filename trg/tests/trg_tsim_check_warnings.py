import basf2 as b2
import generators as ge
import simulation as si
import L1trigger as l1
import b2test_utils as b2u


__offending_warning = '[WARNING] Unknown bitname  { module: TRGGRLProjects }'


def steering_path():
    main = b2.Path()
    main.add_module('EventInfoSetter')
    ge.add_kkmc_generator(main, 'mu-mu+')
    si.add_simulation(main)
    l1.add_tsim(main)
    return main


if __name__ == '__main__':
    with b2u.clean_working_directory():
        log_file = 'log.tmp'
        b2.logging.log_level = b2.LogLevel.WARNING
        b2.logging.add_file(log_file)
        b2u.safe_process(steering_path(), max_event=1)
        offended = False
        with open(log_file, 'r') as log:
            if __offending_warning in log.read():
                offended = True
        if offended:
            bits = []
            with open(log_file, 'r') as log:
                for line in log:
                    if 'bitname =' in line:
                        bits.append(line.split(' ')[2])
            b2.B2FATAL('Check the TRGGRLProjects module for the following undefined bits: ', ' '.join(bits))