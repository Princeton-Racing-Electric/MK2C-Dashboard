from drive_mode import DriveMode

from flask import Flask, Response, render_template, jsonify
from multiprocessing import Process, Value
from datetime import datetime
import time
import math
import random

# =-=-=-=-=-=-= VEHICLE STATUS/SENSOR VALUES =-=-=-=-=-=-=
speed = 0
temperature = 0
soc = 100
odometer = 0
acceleration = 0
throttle = 0
brake = 0
time_current = datetime.now().strftime("%H:%M")


def update_status(speed, temperature, soc, odometer, acceleration, throttle, brake) -> None:
    counter = 0
    while True:
        # Simulate speed using a sine wave for acceleration and deceleration
        # Speed oscillates between 20 and 80
        speed.value = int(50 + 30 * math.sin(counter / 10.0))
        counter += 1

        # Simulate temperature with some randomness to reflect engine/battery heat changes
        # Base temp plus influence of speed
        temperature.value = int(
            20 + random.uniform(-5, 5) + 0.05 * speed.value)

        # Simulate State of Charge (SOC) decreasing gradually, with random fluctuations
        # SOC decreases slowly
        soc.value = max(0, soc.value - random.uniform(0.05, 0.2))

        # Simulate odometer increment based on speed
        # Increment odometer based on speed, e.g., 1 unit per 100 speed value
        odometer.value += speed.value / 100.0

        # Simulate acceleration as a random value that changes when speed changes rapidly
        # Smooth acceleration changes
        acceleration.value = int(
            random.uniform(-3, 3) + 2 * math.sin(counter / 15.0))

        # Simulate throttle and brake values depending on speed changes
        # Throttle fluctuates
        throttle.value = int(
            max(0, 50 + 20 * math.sin(counter / 10.0) - random.uniform(0, 10)))
        # Brake inversely related to throttle
        brake.value = int(
            max(0, 20 - 20 * math.sin(counter / 10.0) + random.uniform(0, 10)))

        if throttle.value > brake.value:
            brake.value = 0
        else:
            throttle.value = 0

        time.sleep(0.1)


# =-=-=-=-=-=-= DRIVE MODES =-=-=-=-=-=-=
driveModes = []
currentDriveMode = 3

modeAcceleration = DriveMode("Acceleration", speed=True, acceleration=True)
modeAutocross = DriveMode("Autocross", speed=True,
                          acceleration=True, throttle=True, brake=True)
modeEndurance = DriveMode("Endurance", speed=True,
                          temperature=True, soc=True, odometer=True)
modeDebug = DriveMode("Debug", speed=True, temperature=True,
                      soc=True, odometer=True, acceleration=True, throttle=True, brake=True)
driveModes.append(modeAcceleration)
driveModes.append(modeAutocross)
driveModes.append(modeEndurance)
driveModes.append(modeDebug)

# =-=-=-=-=-=-= FLASK SETUP =-=-=-=-=-=-=
app = Flask(__name__)


@app.route('/', methods=['GET'])
def dashboard() -> str:
    return render_template('dashboard.html', speed=speed.value, temperature=temperature.value, soc=soc.value, odometer=odometer.value,
                           acceleration=acceleration.value, throttle=throttle.value, brake=brake.value)


@app.route('/status', methods=['GET'])
def status() -> Response:
    return jsonify({'speed': speed.value if driveModes[currentDriveMode].get_speed() else None,
                    'temperature': temperature.value if driveModes[currentDriveMode].get_temperature() else None,
                    'soc': soc.value if driveModes[currentDriveMode].get_soc() else None,
                    'odometer': odometer.value if driveModes[currentDriveMode].get_odometer() else None,
                    'acceleration': acceleration.value if driveModes[currentDriveMode].get_acceleration() else None,
                    'throttle': throttle.value if driveModes[currentDriveMode].get_throttle() else None,
                    'brake': brake.value if driveModes[currentDriveMode].get_brake() else None, })

@app.route('/nextMode', methods=['POST'])
def next_mode() -> Response:
    global currentDriveMode
    currentDriveMode = (currentDriveMode + 1) % len(driveModes)
    return jsonify({'mode': driveModes[currentDriveMode].get_name()})


# =-=-=-=-=-=-= START EVERYTHING =-=-=-=-=-=-=
if __name__ == '__main__':
    speed, temperature, soc, odometer, acceleration, throttle, brake = (
        Value('d', speed),
        Value('d', temperature),
        Value('d', soc),
        Value('d', odometer),
        Value('d', acceleration),
        Value('d', throttle),
        Value('d', brake)
    )
    p = Process(target=update_status, args=(speed, temperature,
                soc, odometer, acceleration, throttle, brake,))
    p.start()
    app.run(debug=True)
    p.join()
