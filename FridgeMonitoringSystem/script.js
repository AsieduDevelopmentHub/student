// App Logic
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

const app = initializeApp(firebaseConfig);
const db  = getDatabase(app);

// Tariff (GHS/kWh) 
const tariffRate = 1.5;

// State
let totalEnergy = 0; // kWh
let currentCost = 0; // GHS
let voltage = 0, current = 0; // V, A

// Gauge Factory
function createGauge(id, maxValue, color){
    const el = document.getElementById(id);
    return new Chart(el, {
    type: "doughnut",
    data: {
        datasets: [{
        data: [0, maxValue],
        backgroundColor: [color, "#e5e7eb"], // slate color for remainder
        borderWidth: 0
        }]
    },
    options: {
        rotation: -90,
        circumference: 180,
        cutout: "70%",
        responsive: true,
        plugins: { legend: { display:false }, tooltip: { enabled:false } }
    }
    });
}

// Create Gauges
const tempGauge = createGauge("tempGauge", 20,  "#ef4444"); // red-500
const voltGauge = createGauge("voltGauge", 300, "#0ea5e9"); // sky-500
const currGauge = createGauge("currGauge", 20,  "#10b981"); // emerald-500

// Cost Graph
const costCard = document.getElementById("costGraph");
const costGraph = new Chart(costCard, {
    type: "line",
    data: {
    labels: [],
    datasets: [{
        label: "Cost (₵)",
        data: [],
        borderColor: "#f59e0b",           // amber-500
        backgroundColor: "rgba(245,158,11,.18)",
        fill: true,
        tension: 0.3,
        pointRadius: 2
    }]
    },
    options: {
    responsive: true,
    plugins: { legend: { display:true } },
    scales: {
        x: { title: { display:true, text:"Time" } },
        y: { title: { display:true, text:"Cost (₵)" }, beginAtZero:true }
    }
    }
});

// Firebase Refs
const tempRef  = ref(db, "Fridge/Sensors/Temperature");
const voltRef  = ref(db, "Fridge/Sensors/Voltage");
const currRef  = ref(db, "Fridge/Sensors/Current");
const relayRef = ref(db, "Fridge/Controls/Relay");
const powerRef = ref(db, "Fridge/Consumption/Wattage");
const kWhRef = ref(db, "Fridge/Consumption/Energy");
const costRef = ref(db, "Fridge/Consumption/Cost");
// const costHistoryRef = ref(db, "Fridge/Consumption/CostHistory");
const historyRef = ref(db, "Fridge/History");
const lastUpdateRef = ref(db, "Fridge/Time/LastUpdate");

// Listeners
onValue(tempRef, snap => {
    const Temperature = Number(snap.val()) || 0;
    const max = 40;
    tempGauge.data.datasets[0].data[0] = Math.min(Math.max(Temperature, 0), max);
    tempGauge.data.datasets[0].data[1] = Math.max(max - Temperature, 0);
    tempGauge.update();
    if (Temperature <= -126) {
        document.getElementById("tempVal").textContent = `N/A`;
    } else {
        document.getElementById("tempVal").textContent = `${Temperature.toFixed(1)} °C`;
    }
});


onValue(voltRef, snap => {
    voltage = Number(snap.val()) || 0;
    const max = 300;
    voltGauge.data.datasets[0].data[0] = Math.min(Math.max(voltage, 0), max);
    voltGauge.data.datasets[0].data[1] = Math.max(max - voltage, 0);
    voltGauge.update();
    document.getElementById("voltVal").textContent = `${voltage.toFixed(1)} V`;
    updatePowerEnergyCost();
});

onValue(currRef, snap => {
    current = Number(snap.val()) || 0;
    const max = 20;
    currGauge.data.datasets[0].data[0] = Math.min(Math.max(current, 0), max);
    currGauge.data.datasets[0].data[1] = Math.max(max - current, 0);
    currGauge.update();
    document.getElementById("currVal").textContent = `${current.toFixed(2)} A`;
    updatePowerEnergyCost();
});

onValue(lastUpdateRef, snap => {
    const lastUpdate = snap.val();
    const lastUpdateEl = document.getElementById("lastUpdate");
    if (lastUpdate) { 
        lastUpdateEl.textContent = `Last Update: ${lastUpdate.toLocaleString()}`;
        lastUpdateEl.style.color = "#311d1dff"; // green for recent
    } else {
        lastUpdateEl.textContent = "Last Update: No updates yet";
        lastUpdateEl.style.color = "#661515ff"; // red for no updates
    }
});

onValue(powerRef, snap => {
    const powerW = Number(snap.val()) || 0;
    document.getElementById("powerVal").textContent = `${powerW.toFixed(2)} W`;
});

onValue(kWhRef, snap => {
    totalEnergy = Number(snap.val()) || 0;
    document.getElementById("energyVal").textContent = `${totalEnergy.toFixed(6)} kWh`;
});

onValue(costRef, snap => {
    currentCost = Number(snap.val()) || 0;
    document.getElementById("costVal").textContent = `₵ ${currentCost.toFixed(4)}`;
});

// Render logs + chart from all history on page load
onValue(historyRef, (snapshot) => {
  const history = snapshot.val();
  const tbody = document.querySelector("#historyTable tbody");

  tbody.innerHTML = ""; // clear old rows
  costGraph.data.labels = [];
  costGraph.data.datasets[0].data = [];

  if (history) {
    // Loop through dates
    for (let date in history) {
      for (let time in history[date]) {
        let record = history[date][time];

        // ---- Table row ----
        let row = document.createElement("tr");
        row.innerHTML = `
          <td>${pickedDate} ${time}</td>
          <td>${record.Voltage}</td>
          <td>${record.Current}</td>
          <td>${record.Energy}</td>
          <td>${record.Cost}</td>
        `;
        tbody.appendChild(row);

        // ---- Add to chart ----
        costGraph.data.labels.push(`${time}`);
        costGraph.data.datasets[0].data.push(Number(record.Cost));
      }
    }

    // Trim chart to last 20 points
    const MAX_POINTS = 10;
    if (costGraph.data.labels.length > MAX_POINTS) {
      costGraph.data.labels = costGraph.data.labels.slice(-MAX_POINTS);
      costGraph.data.datasets[0].data = costGraph.data.datasets[0].data.slice(-MAX_POINTS);
    }
    costGraph.update();
  }
});

onValue(ref(db, "Fridge/History/"), (snapshot) => {
  const logs = snapshot.val();
  costGraph.data.labels = [];
  costGraph.data.datasets[0].data = [];

  if (logs) {
    // Loop through each date
    for (let date in logs) {
      // Loop through each time for that date
      for (let time in logs[date]) {
        let record = logs[date][time];

        costGraph.data.labels.push(`${time}`);
        costGraph.data.datasets[0].data.push(Number(record.Cost));
      }
    }

    // Keep last 10 points only
    const MAX_POINTS = 10;
    if (costGraph.data.labels.length > MAX_POINTS) {
      costGraph.data.labels = costGraph.data.labels.slice(-MAX_POINTS);
      costGraph.data.datasets[0].data = costGraph.data.datasets[0].data.slice(-MAX_POINTS);
    }

    costGraph.update();
  }
});


// Fetch by specific date (filter)
document.getElementById("fetchLogs").addEventListener("click", () => {
  const pickedDate = document.getElementById("datePicker").value;
  const tbody = document.querySelector("#historyTable tbody");

  if (!pickedDate) {
    alert("Please select a date!");
    return;
  }

  onValue(ref(db, "Fridge/History/" + pickedDate), (snapshot) => {
    const logs = snapshot.val();
    tbody.innerHTML = "";
    // costGraph.data.labels = [];
    // costGraph.data.datasets[0].data = [];

    if (logs) {
      for (let time in logs) {
        let record = logs[time];

        // ---- Table row ----
        let row = document.createElement("tr");
        row.innerHTML = `
          <td>${pickedDate} ${time}</td>
          <td>${record.Voltage}</td>
          <td>${record.Current}</td>
          <td>${record.Energy}</td>
          <td>${record.Cost}</td>
        `;
        tbody.appendChild(row);
      }
    } else {
      tbody.innerHTML = `<tr><td colspan="5" style="color:red;">No logs for ${pickedDate}</td></tr>`;
    }
  }, { onlyOnce: true  });
});


const rstMsg = document.getElementById("rst");

// Reset device
window.resetDevice = function() {
    totalEnergy = 0;
    currentCost = 0;
    voltage = 0;
    current = 0;

    tempGauge.data.datasets[0].data[0] = 0;
    voltGauge.data.datasets[0].data[0] = 0;
    currGauge.data.datasets[0].data[0] = 0;

    costGraph.data.labels = [];
    costGraph.data.datasets[0].data = [];
    costGraph.update();

    set(tempRef, 0); // Reset temperature sensor
    set(voltRef, 0); // Reset voltage sensor
    set(currRef, 0); // Reset current sensor
    set(relayRef, 0); // Turn off relay
    set(powerRef, 0); // Reset power
    set(kWhRef, 0); // Reset energy
    set(costRef, 0); // Reset cost
    set(historyRef, null); // Clear history
    set(lastUpdateRef, "No updates yet"); // Reset last update
    if (rstMsg) {
        rstMsg.textContent = "System is Reseting.....";
        rstMsg.style.color = "#10b981"; // green for ready
        setTimeout(() => {
            rstMsg.textContent = "System has been Reset succesfully";
            rstMsg.style.color = "#10b981";
        }, 5000);
    }
    setTimeout(() => {
        set(relayRef, 1); // Turn on relay
        rstMsg.textContent = "System is Ready";
        rstMsg.style.color = "#10b981"; // green for ready
        alert("System has been reset. All values are set to zero.");
        console.log("System reset complete");
    }, 5000);
    setTimeout(() => {
        rstMsg.textContent = "";
    }, 10000);
}

// Relay control
// window.toggleRelay = (state) => set(relayRef, state);
// const msg = document.getElementById("msg");
// if(msg){
//     onValue(relayRef, snap => {
//         const state = snap.val() ? "ON" : "OFF";
//         msg.textContent = `System is ${state}`;
//         msg.style.color = snap.val() ? "#16a34a" : "#dc2626"; // green for ON, red for OFF
//         console.log("Relay snapshot:", snap.val());
//     });
// }

// const relayRef = ref(db, "Fridge/Controls/Relay");   // Relay control

// Toggle relay (true/false or 1/0)
window.toggleRelay = (state) => set(relayRef, state);

// Message element
const msg = document.getElementById("msg");

if (msg) {
  onValue(relayRef, (snap) => {
    let val = snap.val();

    // Normalize: accept 1/0 or true/false
    let isOn = (val === true || val === 1 || val === "1");
    msg.textContent = `System is ${isOn ? "ON" : "OFF"}`;
    msg.style.color = isOn ? "#16a34a" : "#dc2626"; // green/red
    console.log("Relay snapshot:", val);
  });
}

