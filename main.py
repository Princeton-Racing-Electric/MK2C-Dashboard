from drive_mode import DriveMode

from flask import Flask, Response, render_template, jsonify, request, stream_with_context
from multiprocessing import Process, Value, Queue
from datetime import datetime
import time
import math
import random
import can
import json

# =-=-=-=-=-=-= VEHICLE STATUS/SENSOR VALUES =-=-=-=-=-=-=
speed = 0
temperature = 0
soc = 100
odometer = 0
acceleration = 0
throttle = 0
brake = 0
time_current = datetime.now().strftime("%H:%M")
latitude = Value('d', 0.0)
longitude = Value('d', 0.0)
status_queue = Queue()


def update_status(speed, temperature, soc, odometer, acceleration, throttle, brake, latitude, longitude, queue) -> None:
    counter = 0
    # Initialize CAN bus
    bus = can.interface.Bus(interface='slcan', channel='/dev/tty.usbmodem101', bitrate=500000)
    
    while True:
        # Read CAN messages with a very short timeout
        msg = bus.recv(timeout=0.01)
        if msg and msg.arbitration_id == 0x311:  # DR_Lat_Long message
            lat_decimal = int.from_bytes(msg.data[0:4], byteorder='little', signed=True) / 10000000
            long_decimal = int.from_bytes(msg.data[4:8], byteorder='little', signed=True) / 10000000
            latitude.value = lat_decimal
            longitude.value = long_decimal

        # Update other values more frequently
        speed.value = int(50 + 30 * math.sin(counter / 10.0))
        counter += 1
        temperature.value = int(20 + random.uniform(-5, 5) + 0.05 * speed.value)
        soc.value = max(0, soc.value - random.uniform(0.05, 0.2))
        odometer.value += speed.value / 100.0
        acceleration.value = int(random.uniform(-3, 3) + 2 * math.sin(counter / 15.0))
        throttle.value = int(max(0, 50 + 20 * math.sin(counter / 10.0) - random.uniform(0, 10)))
        brake.value = int(max(0, 20 - 20 * math.sin(counter / 10.0) + random.uniform(0, 10)))

        if throttle.value > brake.value:
            brake.value = 0
        else:
            throttle.value = 0

        # Put current status in queue for SSE
        status_data = {
            'speed': speed.value if driveModes[currentDriveMode].get_speed() else None,
            'temperature': temperature.value if driveModes[currentDriveMode].get_temperature() else None,
            'soc': soc.value if driveModes[currentDriveMode].get_soc() else None,
            'odometer': odometer.value if driveModes[currentDriveMode].get_odometer() else None,
            'acceleration': acceleration.value if driveModes[currentDriveMode].get_acceleration() else None,
            'throttle': throttle.value if driveModes[currentDriveMode].get_throttle() else None,
            'brake': brake.value if driveModes[currentDriveMode].get_brake() else None,
            'map': driveModes[currentDriveMode].get_map(),
            'latitude': latitude.value if driveModes[currentDriveMode].get_map() else None,
            'longitude': longitude.value if driveModes[currentDriveMode].get_map() else None,
            'mode': driveModes[currentDriveMode].get_name()
        }
        queue.put(status_data)
        time.sleep(0.1)  # Update at 100Hz


# =-=-=-=-=-=-= DRIVE MODES =-=-=-=-=-=-=
driveModes = []
currentDriveMode = 3

modeAcceleration = DriveMode("Acceleration", speed=True, acceleration=True, odometer=True)
modeAutocross = DriveMode("Autocross", speed=True,
                          acceleration=True, throttle=True, brake=True)
modeEndurance = DriveMode("Endurance", speed=True,
                          temperature=True, soc=True, odometer=True, map=True)
modeDebug = DriveMode("Debug", speed=True, temperature=True,
                      soc=True, odometer=True, acceleration=True, throttle=True, brake=True, map=True)
driveModes.append(modeAcceleration)
driveModes.append(modeAutocross)
driveModes.append(modeEndurance)
driveModes.append(modeDebug)

# =-=-=-=-=-=-= FLASK SETUP =-=-=-=-=-=-=
app = Flask(__name__)


@app.route('/', methods=['GET'])
def dashboard() -> str:
    return render_template('dashboard.html', speed=speed.value, temperature=temperature.value, soc=soc.value, odometer=odometer.value,
                           acceleration=acceleration.value, throttle=throttle.value, brake=brake.value, mode=driveModes[currentDriveMode].get_name())


@app.route('/events')
def events():
    def generate():
        while True:
            # Get data from queue
            data = status_queue.get()
            yield f"data: {json.dumps(data)}\n\n"

    return Response(stream_with_context(generate()), mimetype='text/event-stream')


@app.route('/setMode', methods=['POST'])
def set_mode() -> Response:
    global currentDriveMode    
    data = request.get_json()
    requested_mode = data.get('mode')
    
    # Find the index of the requested mode
    for i, mode in enumerate(driveModes):
        if mode.get_name() == requested_mode:
            currentDriveMode = i
            break
            
    return jsonify({'mode': driveModes[currentDriveMode].get_name()})


@app.route('/api/gps')
def get_gps_data():
    return jsonify({
        'latitude': latitude.value,
        'longitude': longitude.value
    })


# =-=-=-=-=-=-= START EVERYTHING =-=-=-=-=-=-=
if __name__ == '__main__':
    speed, temperature, soc, odometer, acceleration, throttle, brake, latitude, longitude = (
        Value('d', speed),
        Value('d', temperature),
        Value('d', soc),
        Value('d', odometer),
        Value('d', acceleration),
        Value('d', throttle),
        Value('d', brake),
        Value('d', 0.0),
        Value('d', 0.0)
    )
    p = Process(target=update_status, args=(speed, temperature,
                soc, odometer, acceleration, throttle, brake, latitude, longitude, status_queue,))
    p.start()
    app.run(debug=True)
    p.join()
