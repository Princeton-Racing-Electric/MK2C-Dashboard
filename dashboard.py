from flask import Flask, render_template
import random

app = Flask(__name__)

# Home route - Redirect to Acceleration dashboard
@app.route('/')
def home():
    return '''<h1> Home </h1>'''

# Acceleration dashboard route
@app.route('/Acceleration')
def acceleration():
    # Generate dummy data
    speed = random.uniform(0, 200)  # Speed in km/h
    torque = random.uniform(0, 500)  # Torque in Nm
    pressure = random.uniform(20, 35)  # Pressure in PSI
    # Pass data to the template
    return render_template('acceleration.html', speed=speed, torque=torque, pressure=pressure)

# Autocross dashboard route
@app.route('/Autocross')
def autocross():
    # Generate dummy data
    speed = random.uniform(0, 150)
    torque = random.uniform(0, 400)
    pressure = random.uniform(20, 35)
    # Pass data to the template
    return render_template('autocross.html', speed=speed, torque=torque, pressure=pressure)

# Endurance dashboard route
@app.route('/Endurance')
def endurance():
    # Generate dummy data
    speed = random.uniform(0, 180)
    torque = random.uniform(0, 450)
    pressure = random.uniform(20, 35)
    # Pass data to the template
    return render_template('endurance.html', speed=speed, torque=torque, pressure=pressure)

# Run the app
if __name__ == '__main__':
    app.run(debug=True)