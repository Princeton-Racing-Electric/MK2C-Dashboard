// Function to generate random values for each property
let speed = 0;

function updateValues() {
  if (speed < 100) {
    speed += 1;
  } else {
    speed = 0;
  }

  const odometer = (Math.random() * 100).toFixed(1);
  const acceleration = (Math.random() * 10).toFixed(1); // Random acceleration between 0-10 ft/s²
  const throttle = Math.floor(Math.random() * 101); // Random throttle percentage between 0-100%
  const brake = Math.floor(Math.random() * 101); // Random brake percentage between 0-100%
  const soc = Math.floor(Math.random() * 101); // Random state of charge percentage between 0-100%
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
  document.querySelector("#temperature").innerText = `${temperature}°F`;
}

// Update values every second
setInterval(updateValues, 150);
