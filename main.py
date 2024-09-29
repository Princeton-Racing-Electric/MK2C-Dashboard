from flask import Flask, render_template, jsonify
from datetime import datetime
import time
from multiprocessing import Process, Value

speed = 0
temperature = 0
soc = 0
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
    while True:
        if (speed.value < 100):
            speed.value += 1
        else:
            speed.value = 0

        if (temperature.value < 100):
            temperature.value += 1
        else:
            temperature.value = 0

        if (soc.value < 100):
            soc.value += 1
        else:
            soc.value = 0

        if (odometer.value < 100):
            odometer.value += 1
        else:
            odometer.value = 0

        if (acceleration.value < 100):
            acceleration.value += 1
        else:
            acceleration.value = 0

        if (throttle.value < 100):
            throttle.value += 1
        else:
            throttle.value = 0

        if (brake.value < 100):
            brake.value += 1
        else:
            brake.value = 0

        time.sleep(0.1)

if __name__ == '__main__':
    speed, temperature, soc, odometer, acceleration, throttle, brake = Value('i', speed), Value('i', temperature), Value('i', soc), Value('i', odometer), Value('i', acceleration), Value('i', throttle), Value('i', brake)
    p = Process(target=update_status, args=(speed, temperature, soc, odometer, acceleration, throttle, brake,))
    p.start()
    app.run(debug=True)
    p.join()