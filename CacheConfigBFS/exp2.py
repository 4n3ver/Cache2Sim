#!/usr/bin/python3.5

import sys
import math
import subprocess
import re

L1_BUDGET = int(49152)  # 16
L2_BUDGET = int(196608)  # 18
TRACE = ''


def main():
    ext_aat = re.compile("\\(AAT\\) for L1: ([0-9\\.]+)")
    best_AAT = 65535
    best_config = (0, 0, 0, 0, 0, 0, 0)  # cbsvCBS

    config_tried = int(0)
    config_failed = int(0)
    config_invalid = int(0)

    result = open('result_%s.txt' % TRACE.replace('traces/', '').replace('.trace', ''), 'w')
    v = 4
    for C in range(10, 19):
        for B in range(0, C + 1):
            for S in range(0, C - B + 1):
                for c in range(10, min(C + 1, 17)):
                    for b in range(0, min(c + 1, B + 1)):
                        for s in range(0, min(c - b + 1, S + 1)):
                            if is_validConfig(c, b, s, v, C, B, S):
                                result = subprocess.run(
                                    args='./cachesim -c %d -b %d -s %d -v %d -C %d -B %d -S %d < %s' %
                                         (c, b, s, v, C, B, S, TRACE),
                                    shell=True,
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE
                                )
                                if result.returncode == int(0):
                                    config_tried += 1
                                    stats = result.stdout.decode('utf-8')
                                    current_aat = float(ext_aat.search(stats).group(1))
                                    if current_aat < best_AAT:
                                        best_AAT = current_aat
                                        best_config = (c, b, s, v, C, B, S)
                                        print('(^_^)', end='')
                                    else:
                                        print('.', end='')

                                else:
                                    result.write(
                                        '[FAILED CONFIG] ./cachesim -c %d -b %d -s %d -v %d -C %d -B %d -S %d < %s)' %
                                        (c, b, s, v, C, B, S, TRACE))
                                    config_failed += 1
                                    print('F', end='')
                            else:
                                result.write(
                                    '[INVALID_CONFIG] L1=%d L2=%d (./cachesim -c %d -b %d -s %d -v %d -C %d -B %d '
                                    '-S %d < %s)\n' % (cost(c, b, s, v), cost(C, B, S), c, b, s, v, C, B, S, TRACE))
                                config_invalid += 1
                                print('I', end='')
                            sys.stdout.flush()
    sys.stdout.flush()
    result.write('\n\n=====================================================================\n')
    result.write('Best AAT: %f\n' % best_AAT)
    result.write('Best Configuration (c, b, s, v, C, B, S): %s\n' % str(best_config))
    result.write('Simulated Configuration: %d\n', config_tried)
    result.write('Invalid Configuration: %d\n', config_invalid)
    result.write('Failed Configuration: %d\n', config_failed)
    result.write('=====================================================================\n')
    result.close()


def cost(c, b, s, v=0):
    c = int(c)
    b = int(b)
    s = int(s)
    v = int(v)

    main_data = 2 ** c
    main_tagStore = int(math.ceil((2 ** (c - b) * (64 - (c - s) + 2)) / 8))
    vc_data = v * 2 ** b
    vc_tagStore = int(math.ceil((v * (64 - b + 1)) / 8))

    return main_data + main_tagStore + vc_data + vc_tagStore


def is_validConfig(c, b, s, v, C, B, S):
    return cost(c, b, s, v) <= L1_BUDGET and cost(C, B, S) <= L2_BUDGET


if __name__ == "__main__":
    if len(sys.argv) == 2:
        TRACE = str(sys.argv[1])
        print("Going crazy with %s" % TRACE)
        main()
    else:
        print("YOU ARE WRONG!")
