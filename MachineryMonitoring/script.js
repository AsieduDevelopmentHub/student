// Import Firebase functions
        import { initializeApp } from "https://www.gstatic.com/firebasejs/10.12.2/firebase-app.js";
        import { getDatabase, ref, onValue, set } from "https://www.gstatic.com/firebasejs/10.12.2/firebase-database.js";

        // Firebase config
        const firebaseConfig = {
            apiKey: "AIzaSyC0ScXmzo0-O2Vsxjpp2nTDmJkLETobEQg",
            authDomain: "v0ai-real.firebaseapp.com",
            databaseURL: "https://v0ai-real-default-rtdb.firebaseio.com",
            projectId: "v0ai-real",
            storageBucket: "v0ai-real.firebasestorage.app",
            messagingSenderId: "151198275979",
            appId: "1:151198275979:web:54cdffc9acc5b246fb378e",
            measurementId: "G-1828HQE7KH"
        };

        // Initialize Firebase
        const app = initializeApp(firebaseConfig);
        const database = getDatabase(app);

        // Alert thresholds (should match your ESP32 code)
        const thresholds = {
            airQuality: { warning: 650, critical: 800, max: 1400 },
            vibration: { warning: 550, critical: 800, max: 1000 },
            tiltAngle: { warning: 15, critical: 30, max: 45 },
            temperature: { warning: 35, critical: 40, max: 50 },
            humidity: { warning: 85, critical: 90, max: 100 }
        };

        // Current values
        let currentValues = {
            airQuality: 0,
            vibration: 0,
            tiltAngle: 0,
            temperature: 0,
            humidity: 0,
            location: "Unknown"
        };

        // Historical data for charts
        let historicalData = {
            labels: [],
            airQuality: [],
            vibration: [],
            tiltAngle: [],
            temperature: [],
            humidity: []
        };

        // Gauge charts
        const gauges = {};

        // Initialize gauges
        function initGauges() {
            const gaugeConfigs = {
                airQualityGauge: { label: 'Air Quality', unit: 'PPM', threshold: thresholds.airQuality },
                vibrationGauge: { label: 'Vibration', unit: 'Level', threshold: thresholds.vibration },
                tiltAngleGauge: { label: 'Tilt Angle', unit: '°', threshold: thresholds.tiltAngle },
                temperatureGauge: { label: 'Temperature', unit: '°C', threshold: thresholds.temperature },
                humidityGauge: { label: 'Humidity', unit: '%', threshold: thresholds.humidity }
            };

            for (const [id, config] of Object.entries(gaugeConfigs)) {
                const ctx = document.getElementById(id).getContext('2d');
                
                // Set canvas size
                ctx.canvas.width = 300;
                ctx.canvas.height = 150;
                
                gauges[id] = new Chart(ctx, {
                    type: 'doughnut',
                    data: {
                        datasets: [{
                            data: [0, config.threshold.max],
                            backgroundColor: ['#28a745', '#e9ecef'],
                            borderWidth: 0,
                            circumference: 180,
                            rotation: 270,
                            borderRadius: 5
                        }]
                    },
                    options: {
                        cutout: '75%',
                        plugins: {
                            legend: { display: false },
                            tooltip: { enabled: false }
                        },
                        maintainAspectRatio: false,
                        responsive: true
                    }
                });
            }
        }

        // Update gauge value and color
        function updateGauge(gaugeId, value, threshold) {
            const gauge = gauges[gaugeId];
            let color;
            
            if (value < threshold.warning) {
                color = '#28a745'; // Normal - green
            } else if (value < threshold.critical) {
                color = '#ffc107'; // Warning - yellow
            } else {
                color = '#dc3545'; // Critical - red
            }
            
            gauge.data.datasets[0].backgroundColor[0] = color;
            gauge.data.datasets[0].data[0] = Math.min(value, threshold.max);
            gauge.update();
            
            // Update status text
            const statusElement = document.getElementById(gaugeId.replace('Gauge', 'Status'));
            statusElement.className = 'gauge-status ';
            
            if (value < threshold.warning) {
                statusElement.classList.add('status-normal');
                statusElement.textContent = 'Normal';
            } else if (value < threshold.critical) {
                statusElement.classList.add('status-warning');
                statusElement.textContent = 'Warning';
            } else {
                statusElement.classList.add('status-critical');
                statusElement.textContent = 'Critical';
            }
        }

        // Update system status
        function updateSystemStatus() {
            let hasWarning = false;
            let hasCritical = false;
            
            // Check all values against thresholds
            if (currentValues.airQuality >= thresholds.airQuality.critical ||
                currentValues.vibration >= thresholds.vibration.critical ||
                currentValues.tiltAngle >= thresholds.tiltAngle.critical ||
                currentValues.temperature >= thresholds.temperature.critical ||
                currentValues.humidity >= thresholds.humidity.critical) {
                hasCritical = true;
            } else if (currentValues.airQuality >= thresholds.airQuality.warning ||
                currentValues.vibration >= thresholds.vibration.warning ||
                currentValues.tiltAngle >= thresholds.tiltAngle.warning ||
                currentValues.temperature >= thresholds.temperature.warning ||
                currentValues.humidity >= thresholds.humidity.warning) {
                hasWarning = true;
            }
            
            const systemStatus = document.getElementById('systemStatus');
            const systemStatusIndicator = document.getElementById('systemStatusIndicator');
            const systemStatusMessage = document.getElementById('systemStatusMessage');
            
            if (hasCritical) {
                systemStatus.textContent = 'CRITICAL';
                systemStatus.style.color = '#dc3545';
                systemStatusIndicator.className = 'gauge-status status-critical';
                systemStatusIndicator.textContent = 'Critical Issue';
                systemStatusMessage.textContent = 'Critical issues detected. Immediate action required.';
            } else if (hasWarning) {
                systemStatus.textContent = 'WARNING';
                systemStatus.style.color = '#ffc107';
                systemStatusIndicator.className = 'gauge-status status-warning';
                systemStatusIndicator.textContent = 'Warning';
                systemStatusMessage.textContent = 'Warning conditions detected. Please check system.';
            } else {
                systemStatus.textContent = 'NORMAL';
                systemStatus.style.color = '#28a745';
                systemStatusIndicator.className = 'gauge-status status-normal';
                systemStatusIndicator.textContent = 'System OK';
                systemStatusMessage.textContent = 'All systems are operating within normal parameters.';
            }
        }

        // Initialize historical chart with all parameters
        const historicalChart = new Chart(document.getElementById('historicalChart'), {
            type: 'line',
            data: {
                labels: [],
                datasets: [
                    {
                        label: 'Air Quality',
                        data: [],
                        borderColor: 'rgb(75, 192, 192)',
                        backgroundColor: 'rgba(75, 192, 192, 0.1)',
                        tension: 0.3,
                        fill: true,
                        yAxisID: 'y'
                    },
                    {
                        label: 'Vibration',
                        data: [],
                        borderColor: 'rgb(255, 159, 64)',
                        backgroundColor: 'rgba(255, 159, 64, 0.1)',
                        tension: 0.3,
                        fill: true,
                        yAxisID: 'y'
                    },
                    {
                        label: 'Tilt Angle',
                        data: [],
                        borderColor: 'rgb(153, 102, 255)',
                        backgroundColor: 'rgba(153, 102, 255, 0.1)',
                        tension: 0.3,
                        fill: true,
                        yAxisID: 'y1'
                    },
                    {
                        label: 'Temperature',
                        data: [],
                        borderColor: 'rgb(255, 99, 132)',
                        backgroundColor: 'rgba(255, 99, 132, 0.1)',
                        tension: 0.3,
                        fill: true,
                        yAxisID: 'y2'
                    },
                    {
                        label: 'Humidity',
                        data: [],
                        borderColor: 'rgb(54, 162, 235)',
                        backgroundColor: 'rgba(54, 162, 235, 0.1)',
                        tension: 0.3,
                        fill: true,
                        yAxisID: 'y3'
                    }
                ]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                scales: {
                    y: {
                        type: 'linear',
                        display: true,
                        position: 'left',
                        title: {
                            display: true,
                            text: 'Air Quality / Vibration'
                        }
                    },
                    y1: {
                        type: 'linear',
                        display: true,
                        position: 'right',
                        title: {
                            display: true,
                            text: 'Tilt Angle (°)'
                        },
                        grid: {
                            drawOnChartArea: false
                        }
                    },
                    y2: {
                        type: 'linear',
                        display: false,
                        position: 'right',
                        title: {
                            display: true,
                            text: 'Temperature (°C)'
                        },
                        grid: {
                            drawOnChartArea: false
                        }
                    },
                    y3: {
                        type: 'linear',
                        display: false,
                        position: 'right',
                        title: {
                            display: true,
                            text: 'Humidity (%)'
                        },
                        grid: {
                            drawOnChartArea: false
                        }
                    },
                    x: {
                        grid: {
                            display: false
                        }
                    }
                },
                plugins: {
                    legend: {
                        position: 'top',
                        labels: {
                            usePointStyle: true,
                            padding: 20
                        }
                    },
                    tooltip: {
                        mode: 'index',
                        intersect: false
                    }
                }
            }
        });

        // Update the historical chart with new data
        function updateHistoricalChart() {
            // Keep only the last 20 data points
            if (historicalData.labels.length > 20) {
                historicalData.labels.shift();
                historicalData.airQuality.shift();
                historicalData.vibration.shift();
                historicalData.tiltAngle.shift();
                historicalData.temperature.shift();
                historicalData.humidity.shift();
            }
            
            // Update chart data
            historicalChart.data.labels = historicalData.labels;
            historicalChart.data.datasets[0].data = historicalData.airQuality;
            historicalChart.data.datasets[1].data = historicalData.vibration;
            historicalChart.data.datasets[2].data = historicalData.tiltAngle;
            historicalChart.data.datasets[3].data = historicalData.temperature;
            historicalChart.data.datasets[4].data = historicalData.humidity;
            
            historicalChart.update();
        }

        // Fetch historical data from Firebase
        function fetchHistoricalData(date) {
            const tableBody = document.getElementById('historyTableBody');
            tableBody.innerHTML = '<tr><td colspan="7" class="text-center py-4"><div class="spinner-border text-primary" role="status"></div></td></tr>';
            
            const datePath = `Machinery/sensor_data/${date}`;
            const dataRef = ref(database, datePath);
            
            onValue(dataRef, (snapshot) => {
                const data = snapshot.val();
                tableBody.innerHTML = '';
                
                if (!data) {
                    tableBody.innerHTML = '<tr><td colspan="7" class="text-center py-4">No data available for selected date</td></tr>';
                    return;
                }
                
                // Convert object to array and sort by time
                const dataArray = Object.entries(data).map(([time, values]) => ({
                    time,
                    ...values
                })).sort((a, b) => a.time.localeCompare(b.time));
                
                // Populate table
                dataArray.forEach(item => {
                    const row = document.createElement('tr');
                    
                    // Determine overall status for this reading
                    let status = 'normal';
                    if (item.air_quality >= thresholds.airQuality.critical ||
                        item.vibration >= thresholds.vibration.critical ||
                        item.tilt_angle >= thresholds.tiltAngle.critical ||
                        item.temperature >= thresholds.temperature.critical ||
                        item.humidity >= thresholds.humidity.critical) {
                        status = 'critical';
                    } else if (item.air_quality >= thresholds.airQuality.warning ||
                               item.vibration >= thresholds.vibration.warning ||
                               item.tilt_angle >= thresholds.tiltAngle.warning ||
                               item.temperature >= thresholds.temperature.warning ||
                               item.humidity >= thresholds.humidity.warning) {
                        status = 'warning';
                    }
                    
                    let statusBadge = '';
                    if (status === 'normal') {
                        statusBadge = '<span class="status-badge" style="background-color: #28a745; color: white;">Normal</span>';
                    } else if (status === 'warning') {
                        statusBadge = '<span class="status-badge" style="background-color: #ffc107; color: black;">Warning</span>';
                    } else {
                        statusBadge = '<span class="status-badge" style="background-color: #dc3545; color: white;">Critical</span>';
                    }
                    
                    row.innerHTML = `
                        <td>${item.time}</td>
                        <td>${item.air_quality || '--'}</td>
                        <td>${item.vibration || '--'}</td>
                        <td>${item.tilt_angle ? item.tilt_angle.toFixed(1) : '--'}</td>
                        <td>${item.temperature ? item.temperature.toFixed(1) : '--'}</td>
                        <td>${item.humidity ? item.humidity.toFixed(1) : '--'}</td>
                        <td>${statusBadge}</td>
                    `;
                    
                    tableBody.appendChild(row);
                });
            }, (error) => {
                console.error('Error fetching historical data:', error);
                tableBody.innerHTML = '<tr><td colspan="7" class="text-center py-4">Error loading data</td></tr>';
            });
        }

        // Setup real-time listener for latest data
        function setupRealtimeListener() {
            const latestDataRef = ref(database, 'Machinery/latest_reading');
            
            onValue(latestDataRef, (snapshot) => {
                const data = snapshot.val();
                if (data) {
                    // Update current values
                    currentValues.airQuality = data.air_quality || 0;
                    currentValues.vibration = data.vibration || 0;
                    currentValues.tiltAngle = data.tilt_angle || 0;
                    currentValues.temperature = data.temperature || 0;
                    currentValues.humidity = data.humidity || 0;
                    currentValues.location = data.location || "Unknown";
                    
                    // Update gauges
                    updateGauge('airQualityGauge', currentValues.airQuality, thresholds.airQuality);
                    updateGauge('vibrationGauge', currentValues.vibration, thresholds.vibration);
                    updateGauge('tiltAngleGauge', currentValues.tiltAngle, thresholds.tiltAngle);
                    updateGauge('temperatureGauge', currentValues.temperature, thresholds.temperature);
                    updateGauge('humidityGauge', currentValues.humidity, thresholds.humidity);
                    
                    // Update numeric values
                    document.getElementById('airQualityValue').textContent = currentValues.airQuality;
                    document.getElementById('vibrationValue').textContent = currentValues.vibration;
                    document.getElementById('tiltAngleValue').textContent = currentValues.tiltAngle.toFixed(1);
                    document.getElementById('temperatureValue').textContent = currentValues.temperature.toFixed(1);
                    document.getElementById('humidityValue').textContent = currentValues.humidity.toFixed(1);
                    document.getElementById('location').textContent = currentValues.location;
                    
                    // Update system status
                    updateSystemStatus();
                    
                    // Add to historical data for chart
                    const now = new Date();
                    const timeLabel = now.getHours() + ':' + now.getMinutes() + ':' + now.getSeconds();
                    
                    historicalData.labels.push(timeLabel);
                    historicalData.airQuality.push(currentValues.airQuality);
                    historicalData.vibration.push(currentValues.vibration);
                    historicalData.tiltAngle.push(currentValues.tiltAngle);
                    historicalData.temperature.push(currentValues.temperature);
                    historicalData.humidity.push(currentValues.humidity);
                    
                    updateHistoricalChart();
                    
                    // Update last updated time
                    document.getElementById('lastUpdate').textContent = 'Last updated: ' + now.toLocaleTimeString();
                }
            }, (error) => {
                console.error('Error in real-time listener:', error);
            });
        }

        // Initialize the dashboard
        function initDashboard() {
            initGauges();
            setupRealtimeListener();
            
            // Set today's date as default
            const today = new Date();
            const formattedDate = today.toISOString().split('T')[0];
            document.getElementById('dateSelector').value = formattedDate;
            
            // Fetch today's historical data by default
            fetchHistoricalData(formattedDate);
            
            // Add event listener for fetch button
            document.getElementById('fetchHistoryBtn').addEventListener('click', () => {
                const selectedDate = document.getElementById('dateSelector').value;
                if (selectedDate) {
                    fetchHistoricalData(selectedDate);
                }
            });
            
            // Update countdown timer
            let countdown = 10;
            setInterval(() => {
                countdown--;
                if (countdown === 0) countdown = 10;
                document.getElementById('updateTimer').textContent = 'Data updates in real-time';
            }, 1000);
        }

        // Start the dashboard when page loads
        window.onload = initDashboard;