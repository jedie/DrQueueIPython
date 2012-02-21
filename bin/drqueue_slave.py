# -*- coding: utf-8 -*-

"""
DrQueue slave startup script
Copyright (C) 2011 Andreas Schroeder

This file is part of DrQueue.

Licensed under GNU General Public License version 3. See LICENSE for details.
"""


import os, signal, subprocess, sys, platform, time, socket
from collections import deque
from DrQueue import Client as DrQueueClient


if "DRQUEUE_SLAVE" in os.environ:
    SLAVE_IP = os.environ["DRQUEUE_SLAVE"]
else:
    SLAVE_IP = socket.gethostbyname(socket.getfqdn())

SIGTERM_SENT = False
SIGINT_SENT = False
IPENGINE_PID = None


def sig_handler(signum, frame):
    global IPENGINE_PID

    if signum == signal.SIGINT:
        sys.stderr.write("Received SIGINT. Shutting Down.\n")
        global SIGINT_SENT
        if not SIGINT_SENT:
            SIGINT_SENT = True
            if IPENGINE_PID > 0:
                sys.stderr.write("Sending INT to IPython engine.\n")
                os.kill(IPENGINE_PID, signal.SIGINT)
                os.waitpid(IPENGINE_PID, 0)

    if signum == signal.SIGTERM:
        sys.stderr.write("Received SIGTERM. Shutting Down.\n")
        global SIGTERM_SENT
        if not SIGTERM_SENT:
            SIGTERM_SENT = True
            if IPENGINE_PID > 0:
                sys.stderr.write("Sending TERM to IPython engine.\n")
                os.kill(IPENGINE_PID, signal.SIGTERM)
                os.waitpid(IPENGINE_PID, 0)

    sys.exit()


def run_command(command, logfile):
    try:
        p = subprocess.Popen(command, shell=True, stdout=logfile, stderr=subprocess.STDOUT)
    except OSError as e:
        errno, strerror = e.args
        message = "OSError({0}) while executing command: {1}\n".format(errno, strerror)
        logfile.write(message)
        raise OSError(message)
        return False
    return p


def main():
    signal.signal(signal.SIGTERM, sig_handler)
    signal.signal(signal.SIGINT, sig_handler)

    # initialize DrQueue client
    client = DrQueueClient()
    cache_time = 60

    if "DRQUEUE_ROOT" not in os.environ:
        sys.stderr.write("DRQUEUE_ROOT environment variable is not set!\n")
        sys.exit(-1)

    if "IPYTHON_DIR" not in os.environ:
        sys.stderr.write("IPYTHON_DIR environment variable is not set!\n")
        sys.exit(-1)

    if "DRQUEUE_MASTER" not in os.environ:
        sys.stderr.write("DRQUEUE_MASTER environment variable is not set!\n")
        sys.exit(-1)
    else:
        MASTER_IP = os.environ["DRQUEUE_MASTER"]

    global SLAVE_IP
    pid = os.getpid()
    print("Running DrQueue slave on " + SLAVE_IP + " with PID " + str(pid) + ".")
    print("Connecting to DrQueue master at " + MASTER_IP + ".")

    # start IPython engine
    command = "ipengine --url tcp://" + MASTER_IP + ":10101"
    ipengine_logpath = os.path.join(os.environ["DRQUEUE_ROOT"], "logs", "ipengine_" + SLAVE_IP + ".log")
    ipengine_logfile = open(ipengine_logpath, "ab")
    ipengine_daemon = run_command(command, ipengine_logfile)
    global IPENGINE_PID
    IPENGINE_PID = ipengine_daemon.pid
    print("IPython engine started with PID " + str(ipengine_daemon.pid) + ". Logging to " + ipengine_logpath + ".")

    # flush buffers
    ipengine_logfile.flush()
    os.fsync(ipengine_logfile.fileno())
    time.sleep(1)
    # get last line of logfile
    line = deque(open(ipengine_logpath), 1)
    # extract id
    slave_id = int(str(line[0]).split(" ")[-1])
    print("Registered with id " + str(slave_id) + ".")
    time.sleep(6)
    foo = client.ip_client.ids
    comp = client.identify_computer(slave_id, cache_time)

    # remove pool membership if any
    if client.computer_get_pools(comp) != []:
        client.computer_delete(comp)

    # set pool directly after startup
    if "DRQUEUE_POOL" in os.environ:
        client.computer_set_pools(comp, os.environ["DRQUEUE_POOL"].split(","))

    # wait for process to exit
    os.waitpid(ipengine_daemon.pid, 0)


if __name__== "__main__":
    main()

