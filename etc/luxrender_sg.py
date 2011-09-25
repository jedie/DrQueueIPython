# -*- coding: utf-8 -*-

"""
DrQueue render template for Luxrender
Copyright (C) 2011 Andreas Schroeder

This file is part of DrQueue.

Licensed under GNU General Public License version 3. See LICENSE for details.
"""

import os
import DrQueue
from DrQueue import engine_helpers as helper


def run_renderer(env_dict):

    # define external variables as global
    globals().update(env_dict)
    global DRQUEUE_OS
    global DRQUEUE_ETC
    global DRQUEUE_SCENEFILE
    global DRQUEUE_FRAME
    global DRQUEUE_BLOCKSIZE
    global DRQUEUE_ENDFRAME
    global DRQUEUE_LOGFILE

    # range to render
    block = helper.calc_block(DRQUEUE_FRAME, DRQUEUE_ENDFRAME, DRQUEUE_BLOCKSIZE)

    # renderer path/executable
    engine_path = "luxconsole"

    # replace paths on Windows
    if DRQUEUE_OS in ["Windows", "Win32"]:
        DRQUEUE_SCENEFILE = helper.replace_stdpath_with_driveletter(DRQUEUE_SCENEFILE, 'n:')

    command = engine_path + " " + DRQUEUE_SCENEFILE

    # open logfile and write header and command line
    logfile = helper.openlog(DRQUEUE_LOGFILE)
    logfile.write(command + "\n")
    logfile.flush()

    # check scenefile
    helper.check_scenefile(logfile, DRQUEUE_SCENEFILE)

    # run renderer and wait for finish
    ret = helper.run_command(logfile, command)

    # return exit status to IPython
    return helper.return_to_ipython(logfile, ret)

