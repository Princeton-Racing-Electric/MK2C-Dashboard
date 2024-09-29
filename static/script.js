// Function to generate random values for each property
let speed = 0;
let soc = 100;

function updateValues() {
  if (speed < 100) {
    speed += 1;
  } else {
    speed = 0;
  }

  if (soc > 0) {
    soc -= 1;
  } else {
    soc = 100;
  }

  const odometer = (Math.random() * 100).toFixed(1);
  const acceleration = (Math.random() * 10).toFixed(1); // Random acceleration between 0-10 ft/s²
  const throttle = Math.floor(Math.random() * 101); // Random throttle percentage between 0-100%
  const brake = Math.floor(Math.random() * 101); // Random brake percentage between 0-100%
  const temperature = Math.floor(Math.random() * 100); // Random temperature between 0-100°F
  const time = new Date().toLocaleTimeString(); // Current time

  document.querySelector("#speed").innerText = speed;

  document.querySelector(
    "#odometer_acceleration"
  ).innerText = `${odometer} miles · ${acceleration} ft/s²`;

  document.querySelector("#time").innerText = time.substring(
    0,
    time.length - 6
  );

  document.querySelector("#soc").innerText = `${soc}`;
  document.querySelector("#soc").style.color =
    soc <= 20 ? "#BE3838" : "#38be3f";

  const battery_img = document.getElementById("#battery");
  battery_img.src = soc <= 20 ? "/static/img/battery_red.svg" : "/static/img/battery_green.svg";

  document.querySelector("#temperature").innerText = `${temperature}°F`;

  document.querySelector("#throttle").innerText = `${throttle}%`;

  document.querySelector("#brake").innerText = `${brake}%`;
}

// Update values every second
setInterval(updateValues, 150);
