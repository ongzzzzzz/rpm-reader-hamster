// Import the functions you need from the SDKs you need
import { initializeApp } from "https://www.gstatic.com/firebasejs/9.0.0/firebase-app.js";
import { getAnalytics } from "https://www.gstatic.com/firebasejs/9.0.0/firebase-analytics.js";
import { getDatabase, ref, query, limitToLast, onValue } from "https://www.gstatic.com/firebasejs/9.0.0/firebase-database.js";

const firebaseConfig = {
	apiKey: "AIzaSyAZY5x2rJ7Y-yT9_wmUgbcW8z6k8Cwe9nI",
	authDomain: "esp8266-f2775.firebaseapp.com",
	databaseURL: "https://esp8266-f2775.firebaseio.com",
	projectId: "esp8266-f2775",
	storageBucket: "esp8266-f2775.appspot.com",
	messagingSenderId: "370532250396",
	appId: "1:370532250396:web:02fae2c4b1db8a2ec1c0d9",
	measurementId: "G-Z2L3V0T2B6"
};
const app = initializeApp(firebaseConfig);
const analytics = getAnalytics(app);
const db = getDatabase();

let timeZoneOffset = 8 * 60 * 60 * 1000;

// get today's date @midnight 12am
var today = new Date()
today.setHours(0,0,0,0)
today = (today.getTime())/1000;

const rpmDataRef = query(ref(db, 'hamsterRPMdata')
	// , limitToLast(2000)
);

let lower = new Date("2021-09-03").getTime();
let upper = new Date("2021-09-05").getTime();


onValue(rpmDataRef, async (snapshot) => {
	const data = snapshot.val();

	// console.log(data)

	let processedData = [["Time", "RPM"]];
	let epoch; 
	
	Object.keys(data).forEach((time, i) => {
		epoch = new Date(parseInt(time)*1000).getTime();
		if (!(lower <= epoch && epoch <= upper)) return;

		// push twice because nodemcu only pushes data to cloud if got changes
		// so the graph can display correctly, instead of trying to interpolate between datapoints
		if (i != 0) {
			processedData.push(
				[
					new Date((parseInt(time) - 1) * 1000 - timeZoneOffset),
					data[Object.keys(data)[i - 1]]
				]
			)
		}

		// push current data to array
		processedData.push(
			[new Date(parseInt(time) * 1000 - timeZoneOffset), data[time]]
		)
	})

	// console.log(processedData);

	drawChart(processedData);
});



google.charts.load('current', { 'packages': ['corechart'] });

function drawChart(data) {
	var data = google.visualization.arrayToDataTable(data);

	var options = {
		title: 'RPM Data over time',
		legend: { position: 'bottom' }
	};

	var chart = new google.visualization.LineChart(document.getElementById('chart'));

	chart.draw(data, options);
}



// 