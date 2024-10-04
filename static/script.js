const speed = document.getElementById("speed");
const temperature = document.getElementById("temperature");
const soc = document.getElementById("soc");
const battery = document.getElementById("battery");
const odometer_acceleration = document.getElementById("odometer_acceleration");
const throttle = document.getElementById("throttle");
const brake = document.getElementById("brake");
const time = document.getElementById("time");
const throttle_bar = document.getElementById("throttle_bar");
const brake_bar = document.getElementById("brake_bar");

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
        temperature.innerHTML = `${Math.floor(data.temperature)}°F`;
      } else {
        temperature.style.opacity = 0;
      }

      if (data.soc) {
        soc.style.opacity = 1;
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

      if (data.odometer) {
        odometer_acceleration.style.opacity = 1;
        odometer_acceleration.innerHTML = `${data.odometer.toFixed(
          1
        )} miles · ${data.acceleration.toFixed(1)} ft/s²`;
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
    });
  });

  const date = new Date();
  time.innerHTML = `${date.getHours()}:${
    date.getMinutes() < 10 ? "0" : ""
  }${date.getMinutes()}`;
}

setInterval(updateValues, 200);
