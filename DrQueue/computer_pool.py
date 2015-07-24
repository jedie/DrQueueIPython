# -*- coding: utf-8 -*-

"""
DrQueue ComputerPool submodule
Copyright (C) 2011,2012 Andreas Schroeder

This file is part of DrQueue.

Licensed under GNU General Public License version 3. See LICENSE for details.
"""

import os
import logging


log = logging.getLogger(__name__)


try:
    import pymongo
    import bson
except ImportError as err:
    log.debug("Can't import pymongo/bson: %s" % err)
    pymongo = bson = None



def get_queue_pools():
    if pymongo is None:
        raise RuntimeError("pymongo is needed, please install it!")

    connection = pymongo.Connection(os.getenv('DRQUEUE_MONGODB'))
    db = connection['ipythondb']
    pools = db['drqueue_pools']
    return pools


class ComputerPool(dict):
    """Subclass of dict for collecting Pool attribute values."""

    def __init__(self, name, engine_names=None):
        dict.__init__(self)

        if isinstance(engine_names, list):
            raise ValueError("argument is not of type list")

        # mandatory elements
        pool = {
              'name' : name,
              'engine_names' : engine_names,
             }
        self.update(pool)

    @staticmethod
    def store_db(pool):
        """store pool information in MongoDB"""
        pools = get_queue_pools()
        pool_id = pools.insert(pool)
        pool['_id'] = str(pool['_id'])
        return pool_id

    @staticmethod
    def update_db(pool):
        """update pool information in MongoDB"""
        pools = get_queue_pools()
        pool_id = pools.save(pool)
        pool['_id'] = str(pool['_id'])
        return pool_id

    @staticmethod
    def query_db(pool_id):
        """query pool information from MongoDB"""
        pools = get_queue_pools()
        pool = pools.find_one({"_id": bson.ObjectId(pool_id)})
        return pool

    @staticmethod
    def delete_from_db(pool_id):
        """delete pool information from MongoDB"""
        pools = get_queue_pools()
        return pools.remove({"_id": bson.ObjectId(pool_id)})

    @staticmethod
    def query_poolnames():
        """query pool names from MongoDB"""
        pools = get_queue_pools()
        names = []
        for pool in pools.find():
            names.append(pool['name'])
        return names

    @staticmethod
    def query_pool_by_name(pool_name):
        """query pool information from MongoDB by name"""
        pools = get_queue_pools()
        pool = pools.find_one({"name": pool_name})
        return pool

    @staticmethod
    def query_pool_list():
        """query list of pools from MongoDB"""
        pools = get_queue_pools()
        pool_arr = []
        for pool in pools.find():
            pool_arr.append(pool)
        return pool_arr

    @staticmethod
    def query_pool_members(pool_name):
        """query list of members of pool from MongoDB"""
        pools = get_queue_pools()
        pool = pools.find_one({"name": pool_name})
        if pool == None:
            return None
        else:
            return list(pool['engine_names'])
