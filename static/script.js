const speed = document.getElementById('speed');
const temperature = document.getElementById('temperature');
const soc = document.getElementById('soc');
const odometer_acceleration = document.getElementById('odometer_acceleration');
const throttle = document.getElementById('throttle');
const brake = document.getElementById('brake');
const time = document.getElementById('time');
const throttle_bar = document.getElementById('throttle_bar');
const brake_bar = document.getElementById('brake_bar');

async function updateValues() {
  await fetch('/status', {
    method: 'GET',
  }).then((response) => {
    response.json().then((data) => {
      speed.innerHTML = data.speed;
      temperature.innerHTML = `${data.temperature}°F`;
      soc.innerHTML = data.soc;
      odometer_acceleration.innerHTML = `${ data.odometer } miles · ${ data.acceleration } ft/s²`;
      throttle.innerHTML = `${data.throttle}%`;
      brake.innerHTML = `${data.brake}%`;

      // 100% throttle/braking should be height = 84px
      throttle_bar.style.height = `${data.throttle * 0.84}px`;
      brake_bar.style.height = `${data.brake * 0.84}px`;
    });
  });

  const date = new Date();
  time.innerHTML = `${date.getHours()}:${date.getMinutes() < 10 ? '0' : ''}${date.getMinutes()}`;
}

setInterval(updateValues, 50);
