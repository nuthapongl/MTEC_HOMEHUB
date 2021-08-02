var mqtt = require('mqtt');

const MQTT_SERVER = "68.183.224.46";
const MQTT_PORT = "8883";
//if your server don't have username and password let blank.
const MQTT_USER = "tuamang"; 
const MQTT_PASSWORD = "tuamang123";

// Connect MQTT
var client = mqtt.connect({
    host: MQTT_SERVER,
    port: MQTT_PORT,
    username: MQTT_USER,
    password: MQTT_PASSWORD
});

client.on('connect', function () {
  client.subscribe('gate/#', function (err) {
    if (!err) {
      client.publish('presence', 'Hello mqtt')
    }
  })
})

client.on('message', function (topic, message) {
  // message is Buffer
  console.log('message is :' + message.toString())
})