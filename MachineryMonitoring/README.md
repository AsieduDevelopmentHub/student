# ATU Machine Monitoring Dashboard

A real-time IoT-based industrial machine and environmental monitoring system designed for remote supervision, predictive maintenance, and intelligent condition analysis.

---

## Overview

The ATU Machine Monitoring Dashboard is a cloud-connected monitoring platform that combines embedded systems, Firebase cloud infrastructure, and a responsive web dashboard to monitor industrial machines and environmental conditions in real time.

The system continuously streams sensor data from IoT devices to a live dashboard where operators can monitor machine health, environmental safety, and historical trends.

---

## Features

- Real-time sensor monitoring
- Live cloud synchronization using Firebase
- Interactive gauge visualizations
- Historical data tracking and analytics
- Threshold-based warning and critical alerts
- Responsive dashboard interface
- Real-time system health evaluation
- Multi-parameter chart visualization

---

## Monitored Parameters

- Air Quality
- Machine Vibration
- Tilt Angle
- Temperature
- Humidity

---

## System Status Detection

The system automatically evaluates sensor readings and classifies conditions into:

| Status | Description |
|---|---|
| Normal | Safe operating conditions |
| Warning | Elevated values requiring attention |
| Critical | Dangerous conditions requiring immediate action |

---

## Technologies Used

### Frontend
- HTML5
- CSS3
- JavaScript
- Bootstrap 5
- Chart.js
- Font Awesome

### Cloud Services
- Firebase Realtime Database
- Firebase Web SDK

### Embedded Systems
- ESP32
- Environmental Sensors
- Vibration Sensors
- Tilt Sensors
- Temperature & Humidity Sensors

---

## Dashboard Components

### Sensor Monitoring Cards
Each sensor is displayed with:
- Real-time gauge chart
- Live numerical readings
- Dynamic status indicators
- Threshold-based color updates

### Historical Analytics
The dashboard includes:
- Multi-line historical trend charts
- Historical sensor data table
- Date-based filtering system
- Real-time chart updates

### System Status Panel
The dashboard continuously evaluates machine conditions and displays:
- NORMAL
- WARNING
- CRITICAL

based on live sensor data.

---

## System Architecture

```text
+----------------------+
|  ESP32 + Sensors     |
+----------+-----------+
           |
           v
+----------------------+
| Firebase Realtime DB |
+----------+-----------+
           |
           v
+----------------------+
| Web Monitoring UI    |
| Chart.js Dashboard   |
+----------+-----------+
           |
           v
+----------------------+
| Historical Analytics |
| Alert Visualization  |
+----------------------+


## Key Functionalities

- Real-time Firebase data streaming  
- Threshold-based alert detection  
- Interactive Chart.js gauge visualization  
- Historical data logging and retrieval  
- Responsive multi-device dashboard  
- Live environmental and machine monitoring  

---

## Use Cases

- Industrial machine monitoring  
- Predictive maintenance systems  
- Smart factory supervision  
- Environmental condition monitoring  
- Remote equipment diagnostics  
- IoT-based industrial safety systems  

---

## Future Improvements

- AI-powered anomaly detection  
- SMS and email notifications  
- MQTT integration  
- Predictive maintenance analytics  
- Mobile application support  
- User authentication system  
- Cloud report generation  

---

## Developer

### Asiedu Minta Kwaku

Embedded Systems Engineer • IoT Developer • Full-Stack Developer

Focused on building intelligent systems that integrate hardware, software, cloud infrastructure, and real-time monitoring technologies.

---

## License

This project is intended for educational, research, and industrial IoT development purposes.