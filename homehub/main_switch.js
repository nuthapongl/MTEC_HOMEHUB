const express = require('express');
const mqtt = require('mqtt');
const MQTTPattern = require("mqtt-pattern");
const fs = require('fs')
// import function
const init = require('./function/init');
const waiting = require('./function/waiting');
const writing = require('./function/writing');
const update = require('./function/update');

// set variables
var mqttServer = 'mqtt://localhost';
var configFile;
var pattern = "+topic/#data";
var minutes = 15;
var the_interval = minutes * 60 * 1000;
var zigbeeDevice;
var refZigbeeDevice;
// Get a reference to the database service
var client = mqtt.connect(mqttServer);
var incomingData = [];
/*
// init function
init.readFile('./config/config.json').then(init.settingConfig).then(function(result){
  configFile = result;
  // waiting for add device
  waiting.waitForAddDevice(configFile);
  // waiting for reply
  waiting.waitForUserReplied(configFile, client);
  // waiting for read
  waiting.waitForUserRead(configFile, client);
  // waiting for history
  waiting.waitForRequestHistory(configFile);
});
*/
// read list of model Zigbee device
init.readFile('./config/modelDevice.json').then(function (data) {
  console.log(data);
  refZigbeeDevice = JSON.parse(data);
  console.log(refZigbeeDevice[0].deviceType);
});

//read zigbee device w/model from database
init.readFile('./config/device.json').then(function (data) {
  console.log("read device", data);
  zigbeeDevice = JSON.parse(data);
  zigbeeDevice.forEach(function (value) {
    console.log(value.id ," ..",value.deviceType);
  });
});

// set time for update
// setInterval(async function () {
//   await update.deviceDatatoServer(configFile);
// }, the_interval);

// init mqtt subscribe
client.on('connect', function () {
  client.subscribe('bell', function (err) {
    if (!err) {
      console.log(err);
    }
  })
  client.subscribe('gate', function (err) {
    if (!err) {
      console.log(err);
    }
  })
  client.subscribe('gunther32', function (err) {
    if (!err) {
      console.log(err);
    }
  })
  client.subscribe('zigbee2mqtt/#', function (err) {
    if (!err) {
      console.log(err);
    }
  })

});

// waiting mqtt message
client.on('message', async function (topic, message) {
  if (topic.includes('zigbee2mqtt')) {
    console.log(message.toString());
    //var jsonMsg = JSON.parse(message);
    var params = MQTTPattern.exec(pattern, topic);
    var deviceId = params.data[0]
    var deviceType ;
    if (deviceId != "bridge") {
      let jsonMsg = JSON.parse(message);
      zigbeeDevice.forEach(function (value) { 
        if(deviceId == value.id){
          console.log(deviceId ," ..",value.deviceType);
          deviceType = value.deviceType;
          jsonMsg.deviceId = deviceId;
          (async() => {
            await writing.zigbeeLocalData(deviceType, jsonMsg);
          })();
          
        }
      });
    }

    if (deviceId == "bridge") {
      if (params.data[1] == "log") {
        let jsonMsg = JSON.parse(message);
        if (jsonMsg.type == "pairing") {
          // add new device
          if (jsonMsg.message == 'interview_successful') {
            refZigbeeDevice.forEach(function (value) {
              if (value.model == jsonMsg.meta.model) {
                console.log(jsonMsg.meta.friendly_name, "  is ", value.description);

                zigbeeDevice.push({
                  id: jsonMsg.meta.friendly_name,
                  deviceType: value.deviceType,
                })
                // update to database
                try {
                  const data = fs.writeFileSync('./config/device.json', JSON.stringify(zigbeeDevice, null, 2), 'utf-8');
                  //file written successfully
                }
                catch (err) {
                  console.error(err)
                }
                console.log("zigbeeDevice", zigbeeDevice);
              }
            });
          }

        }
      }
    }

  } else if (topic.includes('bell')) {
    var deviceType = 'bell';
    var jsonMsg = JSON.parse(message);
    await writing.espLocalData(deviceType, jsonMsg);
    if (jsonMsg.msg == 'ringbell' || jsonMsg.msg == 'callbell') {
      await update.deviceEventtoServerbyId(configFile, jsonMsg.clientId);
    }
  } else if (topic.includes('gunther32')) {
    var deviceType = 'gunther';
    var jsonMsg = JSON.parse(message);
    console.log(jsonMsg);
    await writing.espLocalData(deviceType, jsonMsg);
    if (jsonMsg.msg == 'fall') {
      await update.deviceEventtoServerbyId(configFile, jsonMsg.clientId);
    }
  }
});

const app = express();
app.get('/', (req, res) => res.send('Hello World!'));
app.listen(3000, () => console.log('Example app listening on port 3000!'));
