#
# Copyright 2021 HiveMQ GmbH
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
import time
import sys
import paho.mqtt.client as paho
from paho import mqtt
from subprocess import PIPE, Popen

publishCount = 0

def get_cpu_temperature():
    """get cpu  using vcgencmd"""
    process = Popen(['vcgencmd', 'measure_temp'], stdout=PIPE)
    output, _error = process.communicate()
    output = output.decode('utf-8')
    return output[output.index('=') + 1:output.rindex("'")]

# setting callbacks for different events to see if it works, print the message etc.
def on_connect(client, userdata, flags, rc, properties=None):
    global publishCount
    #print("CONNACK received with code %s." % rc)

# with this callback you can see if your publish was successful
def on_publish(client, userdata, mid, properties=None):
    global publishCount
    #print("mid: " + str(mid))
    publishCount = publishCount + 1
    if publishCount >= 3 :
        quit()

# print which topic was subscribed to
def on_subscribe(client, userdata, mid, granted_qos, properties=None):
    global publishCount
    #print("Subscribed: " + str(mid) + " " + str(granted_qos))
    

# print message, useful for checking if it was successful
def on_message(client, userdata, msg):
    global publishCount
    #print(msg.topic + " " + str(msg.qos) + " " + str(msg.payload))


# using MQTT version 5 here, for 3.1.1: MQTTv311, 3.1: MQTTv31
# userdata is user defined data of any type, updated by user_data_set()
# client_id is the given name of the client
client = paho.Client(client_id="", userdata=None, protocol=paho.MQTTv5)
client.on_connect = on_connect

# enable TLS for secure connection
client.tls_set(tls_version=mqtt.client.ssl.PROTOCOL_TLS)

# set username and password
#client.username_pw_set("iotMefeTest", "iMT123456")

# connect to HiveMQ Cloud on port 8883 (default for MQTT)
#client.connect("fa82d4f10bdd420baf4489d3436826e2.s1.eu.hivemq.cloud", 8883)
client.connect("broker.emqx.io", 8883)

# setting callbacks, use separate functions like above for better visibility
client.on_subscribe = on_subscribe
client.on_message   = on_message
client.on_publish   = on_publish

mcuTemp = 0
try:
    mcuTemp = get_cpu_temperature()
    pass
except :
    mcuTemp = 0
    pass

# a single publish, this can also be done in loops, etc.
client.publish( "workingRoom/temperature", payload=str( sys.argv[1] ), qos=2 )
client.publish( "workingRoom/humudity", payload=str( sys.argv[2] ),    qos=2 )
client.publish( "workingRoom/mcuTemp", payload= mcuTemp, qos=2 )

# loop_forever for simplicity, here you need to stop the loop manually
# you can also use loop_start and loop_stop
client.loop_forever()

#https://www.emqx.com/en/mqtt/public-mqtt5-broker