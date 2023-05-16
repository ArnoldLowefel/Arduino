import paho.mqtt.client as mqtt
import time


def on_message(client, userdata, message):
    print("message received ", str(message.payload.decode("utf-8")))


broker_address = "test.mosquitto.org"
print("creating new instance")
client = mqtt.Client("Server")
client.on_message = on_message
print("connecting to broker")
client.connect(broker_address)

topic = "soil/humidity/default"
client.loop_start()
print("Subscribing to topic: ", topic)
client.subscribe(topic)
for i in range(100):
    time.sleep(1)

client.loop_stop()
