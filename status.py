from flask import Flask, render_template, jsonify
import pandas as pd
import numpy as np
import os
from datetime import timedelta

app = Flask(__name__)

# Home route
@app.route('/')
def home():
    return render_template('status.html')

# API endpoint to provide data in JSON format
@app.route('/api/data')
def api_data():
    try:
        # Construct the file path to the CSV file
        base_dir = os.path.dirname(os.path.abspath(__file__))
        data_file_path = os.path.join(base_dir, 'data', 'sensor_data.csv')

        # Check if the file exists
        if not os.path.exists(data_file_path):
            return jsonify({'error': 'Data file not found'}), 404

        # Read the CSV file
        data = pd.read_csv(data_file_path)
        data['Timestamp'] = pd.to_datetime(data['Timestamp'])

        # Sort data by Timestamp to ensure proper ordering
        data.sort_values('Timestamp', inplace=True)

        # Calculate time differences in seconds
        data['TimeDiff'] = data['Timestamp'].diff().dt.total_seconds().fillna(0)

        # Calculate speed (assuming 'MotorSpeed' in RPM)
        # You might need to adjust this calculation based on your data
        # For example, converting RPM to km/h if needed
        # Here, we'll assume MotorSpeed is proportional to speed for simplicity
        data['Speed'] = data['MotorSpeed']  # Adjust as necessary

        # Calculate acceleration (delta Speed / delta Time)
        data['Acceleration'] = data['Speed'].diff() / data['TimeDiff']
        data['Acceleration'] = data['Acceleration'].replace([np.inf, -np.inf], np.nan).fillna(0)

        # Calculate lap times if possible (assuming laps are indicated in the data)
        # If you have a 'Lap' column, you can calculate lap times
        # Here, we'll simulate lap times based on every fixed interval or condition
        # For demonstration, we'll assume a lap is completed every N data points

        # Handle NaN values
        data = data.replace({np.nan: None})

        # Filter data for the last 30 minutes
        now = data['Timestamp'].max()
        thirty_minutes_ago = now - timedelta(minutes=30)
        recent_data = data[data['Timestamp'] >= thirty_minutes_ago]

        # Convert Timestamp to string for JSON serialization
        recent_data['Timestamp'] = recent_data['Timestamp'].dt.strftime('%Y-%m-%d %H:%M:%S')

        # Convert data to JSON
        data_json = recent_data.to_dict(orient='records')
        return jsonify(data_json)
    except Exception as e:
        print(f"Error in api_data endpoint: {e}")
        return jsonify({'error': str(e)}), 500

if __name__ == '__main__':
    app.run(debug=True)
