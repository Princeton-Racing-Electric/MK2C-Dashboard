from enum import Enum
import can

# CAN message IDs


class CanID(Enum):
    # -- Front ESP32 Board -- #
    POT_DATA_A = 0x100  # Front two linear pots

    # -- Rear ESP32 Board -- #
    POT_DATA_B = 0x101  # Rear two linear pots

    # -- GPS -- #
    DR_CONFIG = 0x310  # Configuration settings/values + heartbeat
    DR_Lat_Long = 0x311  # Latitude and longitude in decimal degrees
    DR_AltSpeedCOG = 0x312 # Altitude (m), speed (m/s), course over ground (degrees), satellite count, unit mode, PDOP
    DR_Date_Time = 0x313  # Date and time
    DR_IMU_Sensor_Status = 0x314  # IMU sensor status details (off by default)


# Miscellaneous constants
class Constant(Enum):
    ESP32_ADC_Precision = 12
    Max_Pot_Displacement = 3.937
    Word_Size = 32
    Hex_Base = 16
    Word_Select = 0xFFFFFFFF


# Extracts the two 12-bit ADC values from the ESP32 CAN message
def extractESPData(message):
    firstWordToInt = (
        (int((message.data).hex(), Constant.Hex_Base.value)) >> Constant.Word_Size.value)
    secondWordToInt = (
        (int((message.data).hex(), Constant.Hex_Base.value)) & Constant.Word_Select.value)
    return firstWordToInt, secondWordToInt


# Rescales the 12-bit ADC value to the actual potentiometer displacement
def rescale(value):
    return value*Constant.Max_Pot_Displacement.value/(2**Constant.ESP32_ADC_Precision.value)


def parseLatLong(message):
    # Latitude and longitude are signed 32-bit integers representing decimal degrees * 10^7
    lat_decimal = int.from_bytes(message.data[0:4], byteorder='big', signed=True) / 10000000
    long_decimal = int.from_bytes(message.data[4:8], byteorder='big', signed=True) / 10000000
    
    # Convert latitude to degrees, minutes, seconds
    lat_deg = int(abs(lat_decimal))
    lat_min = int((abs(lat_decimal) - lat_deg) * 60)
    lat_sec = round(((abs(lat_decimal) - lat_deg) * 60 - lat_min) * 60, 2)
    lat_dir = 'N' if lat_decimal >= 0 else 'S'
    
    # Convert longitude to degrees, minutes, seconds
    long_deg = int(abs(long_decimal))
    long_min = int((abs(long_decimal) - long_deg) * 60)
    long_sec = round(((abs(long_decimal) - long_deg) * 60 - long_min) * 60, 2)
    long_dir = 'E' if long_decimal >= 0 else 'W'
    
    return (lat_deg, lat_min, lat_sec, lat_dir), (long_deg, long_min, long_sec, long_dir)


# Parses altitude, COG, speed, DR mode, satellite count, and PDOP from DR_AltSpeedCOG message
def parseAltSpeedCOG(message):
    # Altitude: 16-bit value with 0.1m precision, range -100 to +6453.5m
    altitude = int.from_bytes(message.data[0:2], byteorder='big', signed=True) * 0.1
    
    # COG: 16-bit value with 0.1 degree precision, range 0-360 degrees
    cog = int.from_bytes(message.data[2:4], byteorder='big') * 0.1
    if cog > 360:  # Wrap around to valid range
        cog = cog % 360
    
    # Extract speed (bytes 4-5) - 16 bit decimal with 0.1 m/s precision
    speed = int.from_bytes(message.data[4:6], byteorder='big') * 0.1
    
    # Extract DR mode (byte 6 bits 0-2) and satellite count (byte 6 bits 3-7)
    dr_mode = message.data[6] & 0x07  # 0x07 = 0b00000111 to mask first 3 bits

    match dr_mode:
        case 0:
            dr_mode = "No fix"
        case 1:
            dr_mode = "Dead reckoning"
        case 2:
            dr_mode = "2D fix"
        case 3:
            dr_mode = "3D fix"
        case 4:
            dr_mode = "GNSS + dead reckoning"


    satellite_count = (message.data[6] >> 3) & 0x1F  # 0x1F = 0b00011111 to mask 5 bits
    
    # Extract PDOP (byte 7)
    pdop = message.data[7]
    
    return altitude, cog, speed, dr_mode, satellite_count, pdop



# Initialize the CAN bus
bus = can.interface.Bus(
    interface='slcan', channel='/dev/tty.usbmodem101', bitrate=500000, timeout=5)


if (bus):
    print("CAN bus active")
print("{}".format(bus.channel_info))

potA1, potA2 = 0, 0

try:
    for msg in bus:
        # print(msg)
        match msg.arbitration_id:
            # case CanID.POT_DATA_A.value:
            #     potA1, potA2 = extractESPData(msg)
            #     potA1 = rescale(potA1)
            #     potA2 = rescale(potA2)
            #     print(f"pot A1: {potA1}")
            #     print(f"pot A2: {potA2}")
            case CanID.DR_Lat_Long.value:
                lat_tuple, long_tuple = parseLatLong(msg)
                print(f"Latitude: {lat_tuple[0]}° {lat_tuple[1]}' {lat_tuple[2]}\" {lat_tuple[3]}")
                print(f"Longitude: {long_tuple[0]}° {long_tuple[1]}' {long_tuple[2]}\" {long_tuple[3]}")
            case CanID.DR_AltSpeedCOG.value:
                altitude, cog, speed, dr_mode, satellite_count, pdop = parseAltSpeedCOG(msg)
                print(f"Altitude: {altitude:.1f}m")
                print(f"Course over ground: {cog:.1f}°")
                print(f"Speed: {speed:.1f} m/s")
                print(f"DR Mode: {dr_mode}")
                print(f"Satellite count: {satellite_count}")
                print(f"PDOP: {pdop}")
            case _:
                print()
except (KeyboardInterrupt):
    print("Shutting down bus")
    bus.shutdown()


# print(msg)
# try:
#     bus.send(msg)
#     print("Message sent on {}".format(bus.channel_info))
# except can.CanError:
#     print("Message NOT sent")
# bus.shutdown()
