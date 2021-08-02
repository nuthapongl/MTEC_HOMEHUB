var mqtt = require('mqtt');
const fs = require('fs')
const MQTT_SERVER = "68.183.224.46";
const MQTT_PORT = "8883";
//if your server don't have username and password let blank.
const MQTT_USER = "tuamang";
const MQTT_PASSWORD = "tuamang123";
const content = 'Some content!'

fs.appendFile('out.csv', content + '\n', err => {
    if (err) {
        console.error(err)
        return
    }
    //done!
})
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
    var date = new Date();
    var timestamp = date.getTime();
    console.log('amessage is :' + message.toString() + "  "+timestamp);
    const obj = JSON.parse(message);
    if (topic == 'gate/direction') {
        var msg = timestamp + ','+ obj.deviceId +','+ obj.direction + '\n';
        fs.appendFile('direction.csv', msg, function(err) {
            if (err) throw err;
            console.log('Saved!' + msg);
        });
    }
    if (topic == 'gate/status') {
        var msg = timestamp + ','+ obj.deviceId +','+ obj.status + ','+ obj.duration +'\n';
        fs.appendFile('status.csv', msg, function(err) {
            if (err) throw err;
            console.log('Saved status! ' + msg);
        });
    }

})