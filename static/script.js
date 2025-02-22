// Register service worker for tile caching
if ('serviceWorker' in navigator) {
  navigator.serviceWorker.register('/static/sw.js')
    .then(registration => {
      console.log('ServiceWorker registration successful');
    })
    .catch(err => {
      console.error('ServiceWorker registration failed:', err);
    });
}

const speed = document.getElementById("speed");
const speed_unit = document.getElementById("speed_unit");
const temperature = document.getElementById("temperature");
const thermometer = document.getElementById("thermometer");
const soc = document.getElementById("soc");
const battery = document.getElementById("battery");
const odometer_acceleration = document.getElementById("odometer_acceleration");
const throttle = document.getElementById("throttle");
const throttle_bg = document.getElementById("throttle_bg");
const throttle_bar = document.getElementById("throttle_bar");
const brake = document.getElementById("brake");
const brake_bg = document.getElementById("brake_bg");
const brake_bar = document.getElementById("brake_bar");
const time = document.getElementById("time");
const mode = document.getElementById("mode");
const mapContainer = document.getElementById('map');

const ACCEL_DIST = 75; // 75 meters
const ENDUR_DIST = 44000; // 44 kilometers

// Princeton area coordinates
const PRINCETON_CENTER = [40.3500, -74.6525];  // Center around Nassau/Olden intersection

// Initialize the map
let vehicleMap = L.map('map', {
    zoomControl: false,
    attributionControl: false,
    dragging: false,
    touchZoom: false,
    doubleClickZoom: false,
    scrollWheelZoom: false,
    boxZoom: false,
    keyboard: false,
}).setView(PRINCETON_CENTER, 17);  // Zoom level 17 for street-level detail

// Add tile layer with local caching
L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    maxZoom: 19,
    minZoom: 16,
    crossOrigin: true,
    className: 'map-tiles'
}).addTo(vehicleMap);

// Create a custom icon for the vehicle marker
const vehicleIcon = L.divIcon({
    className: 'vehicle-marker',
    html: '<div style="width: 16px; height: 16px; background: #3C81E9; border-radius: 50%; border: 2px solid white;"></div>',
    iconSize: [20, 20],
    iconAnchor: [10, 10]
});

// Create a marker for the vehicle position
let vehicleMarker = L.marker(PRINCETON_CENTER, {icon: vehicleIcon}).addTo(vehicleMap);
let isFirstGPSUpdate = true;

// Add settings elements
const settingsButton = document.getElementById("settings-button");
const settingsMenu = document.getElementById("settings-menu");
const overlay = document.getElementById("overlay");
const exitButton = document.getElementById("exit-button");
const refreshRateSlider = document.getElementById("refresh-rate");
const refreshValue = document.getElementById("refresh-value");
const unitsSelect = document.getElementById("units-select");
let isSettingsOpen = false;
let currentUnits = 'imperial';

// Replace mode change button with mode select handler
const modeSelect = document.getElementById("mode-select");
modeSelect.addEventListener("change", async (event) => {
  const selectedMode = event.target.value;
  await fetch("/setMode", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify({ mode: selectedMode }),
  }).then((response) => {
    response.json().then((data) => {
      console.log(data.mode);
    });
  });
});

function updateValues(data) {
  if (data.speed) {
    speed.style.opacity = 1;
    const speedValue = currentUnits === 'metric' 
      ? Math.round(data.speed * 1.60934)  // mph to kph
      : data.speed;
    speed_unit.innerHTML = currentUnits === 'metric' ? 'KPH' : 'MPH';
    speed.innerHTML = speedValue;
  } else {
    speed.style.opacity = 0;
  }

  if (data.temperature) {
    temperature.style.opacity = 1;
    thermometer.style.opacity = 1;
    const tempValue = currentUnits === 'metric'
      ? Math.floor((data.temperature - 32) * 5/9)  // °F to °C
      : Math.floor(data.temperature);
    const tempUnit = currentUnits === 'metric' ? '°C' : '°F';
    temperature.innerHTML = `${tempValue}${tempUnit}`;
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
    if (data.mode === "Acceleration") {
      odometer_acceleration.innerHTML = `${(
        data.odometer / ACCEL_DIST
      ).toFixed(0)}% · ${data.acceleration.toFixed(0)} ft/s²`;
    } else if (data.mode === "Endurance") {
      odometer_acceleration.innerHTML = `${(
        data.odometer / ENDUR_DIST
      ).toFixed(0)}% · ${data.acceleration.toFixed(0)} ft/s²`;
    } else {
      odometer_acceleration.innerHTML = `${data.odometer.toFixed(
        0
      )} miles · ${data.acceleration.toFixed(0)} ft/s²`;
    }
  } else if (data.odometer) {
    odometer_acceleration.style.opacity = 1;
    if (data.mode === "Acceleration") {
      odometer_acceleration.innerHTML = `${(
        data.odometer / ACCEL_DIST
      ).toFixed(0)}%`;
    } else if (data.mode === "Endurance") {
      odometer_acceleration.innerHTML = `${(
        data.odometer / ENDUR_DIST
      ).toFixed(0)}%`;
    } else {
      odometer_acceleration.innerHTML = `${data.odometer.toFixed(1)} miles`;
    }
  } else if (data.acceleration != null) {
    odometer_acceleration.style.opacity = 1;
    odometer_acceleration.innerHTML = `${data.acceleration.toFixed(
      1
    )} ft/s²`;
  } else {
    odometer_acceleration.style.opacity = 0;
  }

  if (data.throttle != null) {
    throttle.style.opacity = 1;
    throttle.innerHTML = `${data.throttle}%`;
    throttle_bar.style.opacity = 1;
    throttle_bar.style.height = `${data.throttle * 1.3829}px`;
    throttle_bg.style.opacity = 1;
  } else {
    throttle.style.opacity = 0;
    throttle_bar.style.opacity = 0;
    throttle_bg.style.opacity = 0;
  }

  if (data.brake != null) {
    brake.style.opacity = 1;
    brake.innerHTML = `${data.brake}%`;
    brake_bar.style.opacity = 1;
    brake_bar.style.height = `${data.brake * 1.3829}px`;
    brake_bg.style.opacity = 1;
  } else {
    brake.style.opacity = 0;
    brake_bar.style.opacity = 0;
    brake_bg.style.opacity = 0;
  }

  if (data.map && data.latitude !== null && data.longitude !== null) {
    vehicleMarker.setLatLng([data.latitude, data.longitude]);
    vehicleMap.setView([data.latitude, data.longitude], vehicleMap.getZoom());
    mapContainer.style.opacity = 1;
  } else {
    mapContainer.style.opacity = 0.3;
  }

  mode.innerHTML = data.mode;

  const date = new Date();
  time.innerHTML = `${date.getHours()}:${
    date.getMinutes() < 10 ? "0" : ""
  }${date.getMinutes()}`;
}

// Settings menu event listeners
settingsButton.addEventListener("click", () => {
  isSettingsOpen = true;
  settingsMenu.style.right = "0";
  overlay.style.opacity = "1";
  overlay.style.pointerEvents = "auto";
});

exitButton.addEventListener("click", () => {
  isSettingsOpen = false;
  settingsMenu.style.right = "-350px";
  overlay.style.opacity = "0";
  overlay.style.pointerEvents = "none";
});

unitsSelect.addEventListener("change", (event) => {
  currentUnits = event.target.value;
});

// Initialize SSE connection
const eventSource = new EventSource('/events');
eventSource.onmessage = (event) => {
  const data = JSON.parse(event.data);
  updateValues(data);
};

// Handle connection errors
eventSource.onerror = (error) => {
  console.error('SSE connection error:', error);
  // Try to reconnect after 1 second
  setTimeout(() => {
    eventSource.close();
    const newEventSource = new EventSource('/events');
    newEventSource.onmessage = eventSource.onmessage;
    newEventSource.onerror = eventSource.onerror;
  }, 1000);
};
