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
      speed.innerHTML = data.speed;
      temperature.innerHTML = `${Math.floor(data.temperature)}°F`;

      soc.innerHTML = Math.floor(data.soc);
      soc.style.color = data.soc <= 20 ? "#BE3838" : "#38BE56";
      battery.src =
        data.soc <= 20
          ? "/static/img/battery_red.svg"
          : "/static/img/battery_green.svg";

      odometer_acceleration.innerHTML = `${data.odometer.toFixed(
        1
      )} miles · ${data.acceleration.toFixed(1)} ft/s²`;
      throttle.innerHTML = `${data.throttle}%`;
      brake.innerHTML = `${data.brake}%`;

      // 100% throttle/braking should be height = 84px
      throttle_bar.style.height = `${data.throttle * 0.84}px`;
      brake_bar.style.height = `${data.brake * 0.84}px`;
    });
  });

  const date = new Date();
  time.innerHTML = `${date.getHours()}:${
    date.getMinutes() < 10 ? "0" : ""
  }${date.getMinutes()}`;
}

setInterval(updateValues, 200);
