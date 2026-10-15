import paho.mqtt.client as mqtt
from datetime import datetime
import sqlite3
import os


BROKER = os.getenv("MQTT_HOST", "127.0.0.1")
PORT = int(os.getenv("MQTT_PORT", "1883"))

def on_connect(client, userdata, flags, rc):
    print("Connected with result code", rc)

    client.subscribe("moisture")
    client.subscribe("temp")
    client.subscribe("humidity")


def on_message(client, userdata, msg):  
    try:
        val = float(msg.payload.decode())
        topic = msg.topic
        time_now = datetime.now().strftime("%Y-%m-%d %H:%M")
        
        print(f"Received {topic}: {val}")

        connect = sqlite3.connect("water_moisture.db")
        cursor = connect.cursor()
        cursor.execute(
            """
            INSERT INTO sensor_data (time, topic, value)
            VALUES (?, ?, ?)
            """,
            (time_now, topic, val)
        )
        connect.commit()
        connect.close()
    except Exception as e:
        print("Error processing message:", e)

#MQTT stuff
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1)

client.on_connect = on_connect
client.on_message = on_message

print("Connecting...")

client.connect(BROKER, PORT, 60)

client.loop_forever()