var mqtt = require('mqtt');

const MQTT_SERVER = "192.168.1.104";
const MQTT_PORT = "1883";
//if your server don't have username and password let blank.
const MQTT_USER = "node_server";
const MQTT_PASSWORD = "";

//time 24h *3600 + m *60    Sun - sat


// Connect MQTT
var client = mqtt.connect({
    host: MQTT_SERVER,
    port: MQTT_PORT,
    username: MQTT_USER,
    password: MQTT_PASSWORD
});
var options = {
    retain: true
};

client.on('connect', function () {
    // Subscribe any topic
    console.log("MQTT Connect\n");
    client.subscribe('$SYS/broker/log/N', function (err) {
        if (err) {
            console.log(err);
        }
    });
    client.subscribe('#', function (err) {
        if (err) {
            console.log(err);
        }
    });
    // client.publish("Lk3ya3cv/pill", JSON.stringify(pattern), options);
});

var username = "";
// Receive Message and print on terminal
client.on('message', function (topic, message) {

    //console.log(topic);
    if (topic == '$SYS/broker/log/N') {
        if (message.includes("u'")) {
            // console.log("found");
            // console.log("first" + message.indexOf("u'"));
            // console.log("last" + message.indexOf("')."));
            const F_index = message.indexOf("u'") + 2;
            const L_index = message.indexOf("').");
            for (var i = F_index; i < L_index; i++) {
               username += String.fromCharCode(message[i]);
            }
            console.log("New Device Connect to Homehub , Device Id : " + username );
            console.log("Adding to Database ....\n");
            username = "";
        }
    }
    if (topic == 'devices') {
        //JSON
        //console.log(message.toString());
        const editedText = message.slice(0, -1);
        console.log("########################");
        console.log("#  Device Information  #");
        console.log("########################\n\n");
       // console.log(editedText.toString());
        const res = JSON.parse(editedText);
        for(const key in res){
            if(res.hasOwnProperty(key)){
                console.log(`${key} : ${res[key]}`);
            }
        }
    }

    if (topic == 'TestingMQTT_SEND') {
        //JSON
        console.log("\n\nMessage arrived  Topic[" +topic + "] from ESP32 :"+ message.toString());
        console.log("\nGateway reply message on Topic \"TestingMQTT_SEND\" to ESP32 ");
        client.publish("TestingMQTT_RECV", "Reply message From Gateway", options);

    }

});