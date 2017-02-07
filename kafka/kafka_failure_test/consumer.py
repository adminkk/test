#!/usr/bin/env python
# -*- coding: UTF-8 -*-
# ******************************************************
# DESC    : calculate kafka consumer throughout by kafka-python
# AUTHOR  : Alex Stocks
# VERSION : 1.0
# LICENCE : Apache License 2.0
# EMAIL   : alexstocks@foxmail.com
# MOD     : 2016-12-20 21:10
# FILE    : consumer.py
# ******************************************************

import time

from kafka import KafkaConsumer

from calculator import *

brokers="localhost:9092,localhost:19092,localhost:29092"
# brokers="127.0.0.1:9092,127.0.0.1:19092,127.0.0.1:29092"
def python_kafka_consumer_performance():
    consumer = KafkaConsumer(
        # bootstrap_servers=bootstrap_servers.split(","),
        bootstrap_servers=brokers.split(","),
        auto_offset_reset = 'earliest', # start at earliest topic
        # api_version = (0, 10),
        group_id = None # do no offest commit
    )
    msg_consumed_count = 0

    consumer_start = time.time()
    consumer.subscribe([topic])
    for msg in consumer:
        print msg
        msg_consumed_count += 1

        if msg_consumed_count >= msg_count:
            break

    consumer_timing = time.time() - consumer_start
    consumer.close()
    return consumer_timing

_ = python_kafka_consumer_performance()
consumer_timings['python_kafka_consumer'] = python_kafka_consumer_performance()
calculate_thoughput(consumer_timings['python_kafka_consumer'])

# output:
# Processed 1000000 messsages in 25.60 seconds
# 3.73 MB/s
# 39062.33 Msgs/s