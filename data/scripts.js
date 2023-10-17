// External library dependencies
var $ = jQuery;  // Ensure jQuery is accessible as $

// Initial targets for temperature and humidity
var targetTemp = 26;
var targetHumidity = 60;

// Set the initial display values
document.getElementById('targetTemp').innerText = targetTemp;
document.getElementById('targetHumidity').innerText = targetHumidity;

function adjustTargetTemp(delta) {
    targetTemp += delta;
    document.getElementById('targetTemp').innerText = targetTemp;
    updateTargets();
}

function adjustTargetHumidity(delta) {
    targetHumidity += delta;
    document.getElementById('targetHumidity').innerText = targetHumidity;
    updateTargets();
}

function updateTargets() {
    $.get('/setTargets?temp=' + targetTemp + '&humidity=' + targetHumidity);
}

// Update the temperature and humidity values every 3 seconds
setInterval(function(){
    $.get('/data', function(data) {
        $('#temp').text('Temperatur: ' + data.temp + ' C');
        $('#humidity').text('Luftfeuchtigkeit: ' + data.humidity + ' %');
    });
}, 3000);

// Charting for temperature and humidity

var tempChart = new Chart(document.getElementById('tempChart'), {
    type: 'line',
    data: {
        labels: [],
        datasets: [{
            label: 'Temperatur',
            data: [],
            backgroundColor: 'rgba(255, 99, 132, 0.2)',
            borderColor: 'rgba(255, 99, 132, 1)',
            borderWidth: 1
        }]
    },
    options: {
        scales: {
            yAxes: [{
                ticks: {
                    beginAtZero: true
                }
            }]
        }
    }
});

var humidityChart = new Chart(document.getElementById('humidityChart'), {
    type: 'line',
    data: {
        labels: [],
        datasets: [{
            label: 'Luftfeuchtigkeit',
            data: [],
            backgroundColor: 'rgba(54, 162, 235, 0.2)',
            borderColor: 'rgba(54, 162, 235, 1)',
            borderWidth: 1
        }]
    },
    options: {
        scales: {
            yAxes: [{
                ticks: {
                    beginAtZero: true
                }
            }]
        }
    }
});

// Update the charts every 6 seconds
setInterval(function(){
    $.get('/data', function(data) {
        tempChart.data.labels.push(new Date().toLocaleTimeString());
        tempChart.data.datasets[0].data.push(data.temp);
        tempChart.update();

        humidityChart.data.labels.push(new Date().toLocaleTimeString());
        humidityChart.data.datasets[0].data.push(data.humidity);
        humidityChart.update();
    });
}, 6000);
