const speed = document.getElementById("speed");
const temperature = document.getElementById("temperature");
const thermometer = document.getElementById("thermometer");
const soc = document.getElementById("soc");
const battery = document.getElementById("battery");
const odometer_acceleration = document.getElementById("odometer_acceleration");
const throttle = document.getElementById("throttle");
const brake = document.getElementById("brake");
const time = document.getElementById("time");
const throttle_bar = document.getElementById("throttle_bar");
const brake_bar = document.getElementById("brake_bar");
const map = document.getElementById("map");

async function updateValues() {
  await fetch("/status", {
    method: "GET",
  }).then((response) => {
    response.json().then((data) => {
      if (data.speed) {
        speed.style.opacity = 1;
        speed.innerHTML = data.speed;
      } else {
        speed.style.opacity = 0;
      }

      if (data.temperature) {
        temperature.style.opacity = 1;
        thermometer.style.opacity = 1;
        temperature.innerHTML = `${Math.floor(data.temperature)}°F`;
      } else {
        temperature.style.opacity = 0;
        thermometer.style.opacity = 0;
      }

      if (data.soc != null) {
        soc.style.opacity = 1;
        battery.style.opacity = 1;
        soc.innerHTML = Math.floor(data.soc);
        soc.style.color = data.soc <= 20 ? "#BE3838" : "#38BE56";
        battery.src =
          data.soc <= 20
            ? "/static/img/battery_red.svg"
            : "/static/img/battery_green.svg";
      } else {
        soc.style.opacity = 0;
        battery.style.opacity = 0;
      }

      if (data.odometer && data.acceleration != null) {
        odometer_acceleration.style.opacity = 1;
        odometer_acceleration.innerHTML = `${data.odometer.toFixed(
          1
        )} miles · ${data.acceleration.toFixed(1)} ft/s²`;
      } else if (data.odometer) {
        odometer_acceleration.style.opacity = 1;
        odometer_acceleration.innerHTML = `${data.odometer.toFixed(1)} miles`;
      } else if (data.acceleration) {
        odometer_acceleration.style.opacity = 1;
        odometer_acceleration.innerHTML = `${data.acceleration.toFixed(1)} ft/s²`;
      } else {
        odometer_acceleration.style.opacity = 0;
      }

      if (data.throttle) {
        throttle.style.opacity = 1;
        throttle.innerHTML = `${data.throttle}%`;
        throttle_bar.style.opacity = 1;
        throttle_bar.style.height = `${data.throttle * 0.84}px`;
      } else {
        throttle.style.opacity = 0;
        throttle_bar.style.opacity = 0;
      }

      if (data.brake) {
        brake.style.opacity = 1;
        brake.innerHTML = `${data.brake}%`;
        brake_bar.style.opacity = 1;
        brake_bar.style.height = `${data.brake * 0.84}px`;
      } else {
        brake.style.opacity = 0;
        brake_bar.style.opacity = 0;
      }

      if (data.map) {
        map.style.opacity = 1;
      } else {
        map.style.opacity = 0;
      }
    });
  });

  const date = new Date();
  time.innerHTML = `${date.getHours()}:${
    date.getMinutes() < 10 ? "0" : ""
  }${date.getMinutes()}`;
}

async function nextMode() {
  await fetch("/nextMode", {
    method: "POST",
  }).then((response) => {
    response.json().then((data) => {
      console.log(data);
    });
  });
}

document.addEventListener("click", nextMode);

setInterval(updateValues, 200);
