from flask import Flask, render_template, jsonify
from datetime import datetime
import time
from multiprocessing import Process, Value
import math
import random

speed = 0
temperature = 0
soc = 100
odometer = 0
acceleration = 0
throttle = 0
brake = 0
time_current = datetime.now().strftime("%H:%M")

app = Flask(__name__)

@app.route('/', methods=['GET'])
def dashboard():
    return render_template('dashboard.html', speed=speed.value, temperature=temperature.value, soc=soc.value, odometer=odometer.value,
                           acceleration=acceleration.value, throttle=throttle.value, brake=brake.value)

@app.route('/status', methods=['GET'])
def status():
    return jsonify({'speed': speed.value, 'temperature': temperature.value, 'soc': soc.value, 'odometer': odometer.value,
                    'acceleration': acceleration.value, 'throttle': throttle.value, 'brake': brake.value})

def update_status(speed, temperature, soc, odometer, acceleration, throttle, brake):
    counter = 0
    while True:
        # Simulate speed using a sine wave for acceleration and deceleration
        speed.value = int(50 + 30 * math.sin(counter / 10.0))  # Speed oscillates between 20 and 80
        counter += 1

        # Simulate temperature with some randomness to reflect engine/battery heat changes
        temperature.value = int(20 + random.uniform(-5, 5) + 0.05 * speed.value)  # Base temp plus influence of speed

        # Simulate State of Charge (SOC) decreasing gradually, with random fluctuations
        soc.value = max(0, soc.value - random.uniform(0.05, 0.2))  # SOC decreases slowly

        # Simulate odometer increment based on speed
        odometer.value += speed.value / 100.0  # Increment odometer based on speed, e.g., 1 unit per 100 speed value

        # Simulate acceleration as a random value that changes when speed changes rapidly
        acceleration.value = int(random.uniform(-3, 3) + 2 * math.sin(counter / 15.0))  # Smooth acceleration changes

        # Simulate throttle and brake values depending on speed changes
        throttle.value = int(max(0, 50 + 20 * math.sin(counter / 10.0) - random.uniform(0, 10)))  # Throttle fluctuates
        brake.value = int(max(0, 20 - 20 * math.sin(counter / 10.0) + random.uniform(0, 10)))  # Brake inversely related to throttle

        if throttle.value > brake.value:
            brake.value = 0
        else:
            throttle.value = 0

        time.sleep(0.1)

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
    p = Process(target=update_status, args=(speed, temperature, soc, odometer, acceleration, throttle, brake,))
    p.start()
    app.run(debug=True)
    p.join()