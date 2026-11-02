#!/usr/bin/env python3

from mosq_test_helper import *

from broker_config import BrokerConfig, ListenerConfig, PluginConfig
from mosquitto_broker import MosquittoBroker


def do_test(proto_ver):
    port = mosq_test.get_port()
    broker_config = BrokerConfig(
        listeners=[ListenerConfig(port=port)],
        plugins=[PluginConfig(path=mosq_paths.test_plugin('plugin_evt_persist_will'))],
        allow_anonymous=True,
    )
    with MosquittoBroker(config=broker_config):
        sub_sock = mosq_test.sub_helper(port, 'evt/will/#')
        try:
            connect_packet = mqtt_packets.gen_connect(
                'will-client', proto_ver=proto_ver,
                will_topic='will/topic', will_payload=b'will payload',
            )
            connack_packet = mqtt_packets.gen_connack(rc=0, proto_ver=proto_ver)
            sock = mosq_test.do_client_connect(connect_packet, connack_packet, port=port)
            try:
                mosq_test.expect_packet(
                    sub_sock, 'Will addition',
                    mqtt_packets.gen_publish('evt/will/add', qos=0, payload='will-client'),
                )
                sock.send(mqtt_packets.gen_disconnect(proto_ver=proto_ver))
                mosq_test.expect_packet(
                    sub_sock, 'Will deletion',
                    mqtt_packets.gen_publish('evt/will/delete', qos=0, payload='will-client'),
                )
            finally:
                sock.close()
        finally:
            sub_sock.close()


do_test(proto_ver=4)
do_test(proto_ver=5)
