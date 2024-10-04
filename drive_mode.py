class DriveMode:
    def __init__(self, name, speed=False, temperature=False, soc=False, odometer=False, acceleration=False, throttle=False, brake=False, map=False):
        self.name = name
        self.speed = speed
        self.temperature = temperature
        self.soc = soc
        self.odometer = odometer
        self.acceleration = acceleration
        self.throttle = throttle
        self.brake = brake
        self.map = map

    def get_name(self) -> str:
        return self.name

    def get_speed(self) -> bool:
        return self.speed

    def get_temperature(self) -> bool:
        return self.temperature

    def get_soc(self) -> bool:
        return self.soc

    def get_odometer(self) -> bool:
        return self.odometer

    def get_acceleration(self) -> bool:
        return self.acceleration

    def get_throttle(self) -> bool:
        return self.throttle

    def get_brake(self) -> bool:
        return self.brake
    
    def get_map(self) -> bool:
        return self.map