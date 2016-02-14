#!/usr/bin/python3.5

import concurrent.futures
import threading
import sys
import math
import subprocess
import re

L1_BUDGET = int(49152)  # 16
L2_BUDGET = int(196608)  # 18
TRACE = ''
ext_aat = re.compile("\\(AAT\\) for L1: ([0-9\\.]+)")
lock = threading.Lock()

result = None

# requires sync
config_tried = int(0)
config_failed = int(0)
config_invalid = int(0)
best_AAT = 65535
best_config = (0, 0, 0, 0, 0, 0, 0)  # cbsvCBS


def main():
    with concurrent.futures.ThreadPoolExecutor(max_workers=8) as executor:
        for C in range(10, 19):
            for B in range(0, C + 1):
                for S in range(0, C - B + 1):
                    for c in range(10, min(C + 1, 17)):
                        for b in range(0, min(c + 1, B + 1)):
                            for s in range(0, min(c - b + 1, S + 1)):
                                exe = executor.submit(
                                    fn=simulate,
                                    args=(c, b, s, 4, C, B, S)
                                )
        executor.shutdown()
    sys.stdout.flush()
    result.write('\n\n=====================================================================\n')
    result.write('Best AAT: %f\n' % best_AAT)
    result.write('Best Configuration (c, b, s, v, C, B, S): %s\n' % str(best_config))
    result.write('Simulated Configuration: %d\n' % config_tried)
    result.write('Invalid Configuration: %d\n' % config_invalid)
    result.write('Failed Configuration: %d\n' % config_failed)
    result.write('=====================================================================\n')
    result.close()


def simulate(arg_c, arg_b, arg_s, arg_v, arg_C, arg_B, arg_S):
    global config_tried, config_failed, config_invalid, best_AAT, best_config
    if is_validConfig(arg_c, arg_b, arg_s, arg_v, arg_C, arg_B, arg_S):
        out = subprocess.run(
            args='./cachesim -c %d -b %d -s %d -v %d -C %d -B %d -S %d < %s' %
                 (arg_c, arg_b, arg_s, arg_v, arg_C, arg_B, arg_S, TRACE),
            shell=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        if out.returncode == int(0):
            lock.acquire()
            config_tried += 1
            stats = out.stdout.decode('utf-8')
            current_aat = float(ext_aat.search(stats).group(1))
            if current_aat < best_AAT:
                best_AAT = current_aat
                best_config = (arg_c, arg_b, arg_s, arg_v, arg_C, arg_B, arg_S)
                print('(^_^)', end='')
            else:
                print('.', end='')
            lock.release()
        else:
            lock.acquire()
            result.write(
                '[FAILED CONFIG] ./cachesim -c %d -b %d -s %d -v %d -C %d -B %d -S %d < %s)' %
                (arg_c, arg_b, arg_s, arg_v, arg_C, arg_B, arg_S, TRACE))
            config_failed += 1
            lock.release()
            print('F', end='')
    else:
        lock.acquire()
        result.write(
            '[INVALID_CONFIG] L1=%d L2=%d (./cachesim -c %d -b %d -s %d -v %d -C %d -B %d '
            '-S %d < %s)\n' % (
                cost(arg_c, arg_b, arg_s, arg_v), cost(arg_C, arg_B, arg_S), arg_c, arg_b, arg_s, arg_v, arg_C, arg_B,
                arg_S, TRACE))
        config_invalid += 1
        lock.release()
        print('I', end='')
    sys.stdout.flush()


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
        result = open('result_%s.txt' % TRACE.replace('traces/', '').replace('.trace', ''), 'w')
        main()
    else:
        print("YOU ARE WRONG!")
