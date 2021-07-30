const express = require('express');
const firebase = require('firebase-admin');
const fs = require('fs');
const ping = require('ping');
var mqtt = require('mqtt');
const axios = require('axios');
var moment = require('moment');
var MQTTPattern = require("mqtt-pattern");
const uuid = require('uuid');
const serviceAccount = require('./connectedpeaceofmind-817d4-firebase-adminsdk-38alx-fcd18fc0f0.json');
const DeviceOccupancyController = require('./controller/deviceOccupancyController');
const DeviceLightController = require('./controller/deviceLightController');
const DeviceDoorController = require('./controller/deviceDoorController');
const DeviceVibrateController = require('./controller/deviceVibrateController');
const DeviceCodeController = require('./controller/deviceCodeController');

// set variables
const databaseURL = 'https://connectedpeaceofmind-817d4.firebaseio.com/';
var mqttServer = 'mqtt://localhost';
var jwtToken = '';
var headers;
var configFile;
var interval;
var count = 0; 
var isOutGate = true;
var countIn = 0;
var pattern = "+topic/#data";
var config = {
  credential: firebase.credential.cert(serviceAccount),
  databaseURL: databaseURL
};

// Get a reference to the database service
firebase.initializeApp(config);
var database = firebase.database();
var client  = mqtt.connect(mqttServer);

// init function
init();

async function init() {
  try {
    await readFile('config.json').then(settingConfig);
  }
  catch(err) {
    console.log(err);
  }
}

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
})

client.on('message', async function (topic, message) {

  // message is Buffer
  if(topic.includes('bell')) {
    var jsonMsg = JSON.parse(message);
    console.log(jsonMsg);
    var deviceId = jsonMsg.clientId;
    const isDeviceSub = await DeviceCodeController.findDeviceCode(deviceId);
    if (isDeviceSub) {
      var replyMsg = {"clientId": deviceId};
      if(jsonMsg.msg == 'connection_btn') {
        console.log('check internet status with button');
        ping.sys.probe('8.8.8.8', function(active) {
          if(active) {
            replyMsg.msg = 'ready_btn';
            if (jsonMsg.battery == "normal") {
              updateDeviceStatus(deviceId,2,2);
            } else {
              updateDeviceStatus(deviceId,1,2);
            }
          }
          else {
            replyMsg.msg = 'notready_btn';
          }
        })
      }
      else if(jsonMsg.msg == 'connection') {
        console.log('check internet status');
        ping.sys.probe('8.8.8.8', function(active) {
          if(active) {
            replyMsg.msg = 'ready';
            if (jsonMsg.battery == "normal") {
                updateDeviceStatus(deviceId,2,2);
            } 
            else {
              updateDeviceStatus(deviceId,1,2);
            }
          } else {
            replyMsg.msg = 'notready';
          }
        })
      }
      else if(jsonMsg.msg == 'ringbell') {
        replyMsg.msg = 'otw';
        writeBellData(deviceId, 0);
        }
      else if(jsonMsg.msg == 'callbell') {
        replyMsg.msg = 'otw';
        writeBellData(deviceId, 1);
      }
      client.publish('homehub', JSON.stringify(replyMsg));
    }
  }
  else if(topic.includes('gate')) {
    var jsonMsg = JSON.parse(message);
    console.log(jsonMsg);
    var deviceId = jsonMsg.clientId;
    const isDeviceSub = await DeviceCodeController.findDeviceCode(deviceId);
    if (isDeviceSub) {
      var replyMsg = {"clientId": deviceId};
      if(jsonMsg.msg === 'in') {
        countIn++;
        isOutGate = false
        if (!isOutGate && countIn == 1) {
          interval = setInterval(increaseTime, 1000, deviceId);
        } else if (!isOutGate && countIn > 1) {
          clearInterval(interval);
          count = 0;
        }
        console.log('in');
      }
      else if (jsonMsg.msg === 'out') {
        if (countIn != 0) {
          countIn--;
          if (countIn == 0) {
            clearInterval(interval);
            count = 0;
            isOutGate = true;
          } else if (!isOutGate && countIn == 1) {
            interval = setInterval(increaseTime, 1000, deviceId);
          }
        }
        console.log('out');
      }
    }
  }
  else if (topic.includes('gunther32')) {
    var jsonMsg = JSON.parse(message);
    console.log(jsonMsg);
    var deviceId = jsonMsg.clientId;
    const isDeviceSub = await DeviceCodeController.findDeviceCode(deviceId);
    if (isDeviceSub) {
      var replyMsg = {"clientId": deviceId}
      if(jsonMsg.msg == 'fall') {
        replyMsg.msg = 'otw';
        writeGuntherData(deviceId, 0);
        client.publish('homehub', JSON.stringify(replyMsg));
      } else if(jsonMsg.msg == 'start') {
        if (jsonMsg.battery == "normal") {
          updateDeviceStatus(deviceId,2,2);
        } else {
          updateDeviceStatus(deviceId,1,2);
        }
      } else if(jsonMsg.msg == 'moving') {
        if (jsonMsg.battery == "normal") {
          updateDeviceStatus(deviceId,2,2);
        } else {
          updateDeviceStatus(deviceId,1,2);
        }
      } else if(jsonMsg.msg == 'not_moving') {
        if (jsonMsg.battery == "normal") {
          updateDeviceStatus(deviceId,2,2);
        } else {
          updateDeviceStatus(deviceId,1,2);
        }
      }
    }
  }
  else if (topic.includes('zigbee2mqtt')) {
    var params = MQTTPattern.exec(pattern, topic);
    console.log(topic);
    console.log(params);
    if (params.data[0] === 'occupancy') {
      console.log(message.toString());
      var jsonMsg = JSON.parse(message);
      jsonMsg.eventId = uuid.v4();
      jsonMsg.deviceId = "7cbad539-3c5d-4989-b5ed-e5b1c6aac407";
      const isDeviceSub = await DeviceCodeController.findDeviceCode(jsonMsg.deviceId);
      if (isDeviceSub) {
        writeDeviceLocalData(params.data[0], jsonMsg);
      }
    } 
    else if (params.data[0] === 'light') {
      console.log(message.toString());
      var jsonMsg = JSON.parse(message);
      jsonMsg.eventId = uuid.v4();
      jsonMsg.deviceId = "eca06ac5-d04c-4fcc-9d35-e71c27a4d3a5";
      const isDeviceSub = await DeviceCodeController.findDeviceCode(jsonMsg.deviceId);
      if (isDeviceSub) {
        writeDeviceLocalData(params.data[0], jsonMsg);
      }
    } 
    else if (params.data[0] === 'door') {
      console.log(message.toString());
      var jsonMsg = JSON.parse(message);
      jsonMsg.eventId = uuid.v4();
      jsonMsg.deviceId = "93dedffc-629d-4e08-ac27-9284b07ff25e";
      const isDeviceSub = await DeviceCodeController.findDeviceCode(jsonMsg.deviceId);
      if (isDeviceSub) {
        writeDeviceLocalData(params.data[0], jsonMsg);
      }
    }
    else if (params.data[0] === 'vibrate') {
      console.log(message.toString());
      var jsonMsg = JSON.parse(message);
      jsonMsg.eventId = uuid.v4();
      jsonMsg.deviceId = "ba11bdd7-174a-4a89-b06d-b5b3d2b2c421";
      const isDeviceSub = await DeviceCodeController.findDeviceCode(jsonMsg.deviceId);
      if (isDeviceSub) {
        writeDeviceLocalData(params.data[0], jsonMsg);
      }
    }
  }
})

var minutes = 15, the_interval = minutes * 60 * 1000;
setInterval(async function() {
  const deviceCodes = await deviceCodeController.findAllDeviceCodes();
  if (deviceCodes != null) {
    for(i = 0; i < deviceCodes.length; i++) {
      if (deviceCodes[i].deviceType == "occupancy") {
        const currentOccupancyData = await DeviceOccupancyController.currentDeviceOccupancy(deviceCodes[i].deviceId);
        console.log(currentOccupancyData);
      } else if (deviceCodes[i].deviceType == "door") {
        const currentDoorData = await DeviceDoorController.currentDeviceDoor(deviceCodes[i].deviceId);
        console.log(currentDoorData);
        const result = await DeviceDoorController.updateDeviceDoorToServer(configFile.hostname, configFile.DEVICESTATUS_API, headers, currentDoorData);
        console.log(result);
      } else if (deviceCodes[i].deviceType == "light") {
        const currentLightData = await DeviceLightController.currentDeviceLight(deviceCodes[i].deviceId);
        console.log(currentLightData);
      } else if (deviceCodes[i].deviceType == "vibrate") {
        const currentVibrateData = await DeviceVibrateController.currentDeviceVibrate(deviceCodes[i].deviceId);
        console.log(currentVibrateData);
      } 
    }
  }
}, the_interval);

function readFile(fileName) {
  return new Promise((resolve, reject) => {
    fs.readFile(fileName, 'utf8', async function (error, data) {
      if (error) {
        return reject(error)
      } else {
        resolve(data);
      }
    })
  });
}

async function settingConfig(data) {
  if(data) {
    configFile = JSON.parse(data);
    const serialNo = {
      serial: configFile.uuid,
      device: 'homehub'
    }
    var result = await postToServer(configFile.hostname, configFile.LOGIN_API, serialNo, false);
    jwtToken = result.token;
    console.log(jwtToken);
    headers = {
      'Content-Type': 'application/json',
      'Authorization': 'Bearer '+ jwtToken,
    }
    waitForReply();
    waitForBellAck();
    waitForRequestUpdate();
    waitForAddDevice();
  }
}

async function updateDeviceStatus(deviceId, deviceBattery, deviceAvailable) {
  var deviceStatus = {
    deviceBattery: deviceBattery, // message 0 = falling
    deviceAvailable: deviceAvailable,
    deviceId: deviceId, //token
  }; 
  var result = await postToServer(configFile.hostname, configFile.DEVICESTATUS_API, JSON.stringify(deviceStatus), true);
  console.log(result);
}

async function postToServer(hostname, api, body, isHeader) {
  var response;
  console.log(body)
  if(isHeader) {
    response = await axios.post(hostname + api, body, {headers: headers});
  }
  else {
    response = await axios.post(hostname + api, body);
  }
  if(response) {
    return response.data;
  }
  return 'cannot reach host';
}

async function getFromServer(hostname, api, body) {
  var response = await axios.get(hostname + api + body, {headers: headers});
  if(response) {
    return response.data;
  }
  return 'cannot reach host';
}

const app = express()
app.get('/', (req, res) => res.send('Hello World!'))
app.listen(3000, () => console.log('Example app listening on port 3000!'))