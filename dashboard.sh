#!/bin/bash

cd /home/pre/MK2C-Dashboard
source venv/bin/activate
python3 main.py &
sleep 15
firefox --kiosk "http://localhost:5000" &