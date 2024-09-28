from flask import Flask, render_template
from datetime import datetime

app = Flask(__name__)

@app.route('/')
def dashboard():
    speed = 50
    temperature = 25
    soc = 80
    odometer = 5
    acceleration = 2
    throttle = 77
    brake = 19
    time_current = datetime.now().strftime("%H:%M")
    return render_dashboard(speed, temperature, soc, odometer, acceleration, throttle, brake, time_current)

def render_dashboard(speed, temperature, soc, odometer, acceleration, throttle, brake, time_current):
    return render_template('dashboard.html', speed=speed, temperature=temperature, soc=soc, odometer=odometer,
                           acceleration=acceleration, throttle=throttle, brake=brake, time=time_current)