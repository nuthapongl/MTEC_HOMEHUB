const DeviceOccupancyController = require('../controller/deviceOccupancyController');
const DeviceLightController = require('../controller/deviceLightController');
const DeviceDoorController = require('../controller/deviceDoorController');
const DeviceVibrateController = require('../controller/deviceVibrateController');
const DeviceBellController = require('../controller/deviceBellController');
const DeviceGuntherController = require('../controller/deviceGuntherController');
const DeviceSwitchController = require('../controller/deviceSwitchController');
const DeviceController = require('../controller/deviceController');
const uuid = require('uuid');
const ping = require('ping');

async function zigbeeLocalData(deviceType, jsonMsg) {
  if (deviceType === 'occupancy') {
    jsonMsg.deviceId = "7cbad539-3c5d-4989-b5ed-e5b1c6aac407";
    const isDeviceSub = await DeviceController.findDeviceCode(jsonMsg.deviceId);
    if (isDeviceSub) {
      jsonMsg.eventId = uuid.v4();
      var message = await DeviceOccupancyController.createDeviceOccupancy(jsonMsg);
      console.log(message);
    }
  } else if (deviceType === 'light') {
    jsonMsg.deviceId = "eca06ac5-d04c-4fcc-9d35-e71c27a4d3a5";
    const isDeviceSub = await DeviceController.findDeviceCode(jsonMsg.deviceId);
    if (isDeviceSub) {
      jsonMsg.eventId = uuid.v4();
      var message = await DeviceLightController.createDeviceLight(jsonMsg);
      console.log(message);
    }
  } else if (deviceType === 'door') {
    jsonMsg.deviceId = "93dedffc-629d-4e08-ac27-9284b07ff25e";
    const isDeviceSub = await DeviceController.findDeviceCode(jsonMsg.deviceId);
    if (isDeviceSub) {
      jsonMsg.eventId = uuid.v4();
      var message = await DeviceDoorController.createDeviceDoor(jsonMsg);
      console.log(message);
    }
  } else if (deviceType === 'vibrate') {
    jsonMsg.deviceId = "ba11bdd7-174a-4a89-b06d-b5b3d2b2c421";
    const isDeviceSub = await DeviceController.findDeviceCode(jsonMsg.deviceId);
    if (isDeviceSub) {
      jsonMsg.eventId = uuid.v4();
      var message = await DeviceVibrateController.createDeviceVibrate(jsonMsg);
      console.log(message);
    }
  } else if (deviceType === 'switch') {
    console.log("Device id is", jsonMsg.deviceId);
    const isDeviceSub = await DeviceController.findDeviceCode(jsonMsg.deviceId);
    if (isDeviceSub) {
      jsonMsg.eventId = uuid.v4();
      var message = await DeviceSwitchController.createDeviceSwitch(jsonMsg);
      console.log(message);
    }
  }

}

async function espLocalData(deviceType, jsonMsg) {
  if (deviceType === 'bell') {
    var bellData = { "eventId": uuid.v4() };
    bellData.deviceId = jsonMsg.clientId;
    const isDeviceSub = await DeviceController.findDeviceCode(bellData.deviceId);
    if (isDeviceSub) {
      if (jsonMsg.msg === 'connection_btn' || jsonMsg.msg === 'connection') {
        bellData.deviceBattery = (jsonMsg.battery === "normal") ? 1 : 0;
        bellData.deviceMessage = jsonMsg.msg;
        bellData.deviceEvent = 0; // deviceEvent = 0 = update status
        bellData.replyEvent = null;
        ping.sys.probe('8.8.8.8', async function (active) {
          if (active) {
            bellData.deviceAvailable = 1;
            console.log(await DeviceBellController.createDeviceBell(bellData));
          } else {
            bellData.deviceAvailable = 0;
            console.log(await DeviceBellController.createDeviceBell(bellData));
          }
        })
      } else if (jsonMsg.msg === 'ringbell' || jsonMsg.msg === 'callbell') {
        bellData.deviceBattery = (jsonMsg.battery === "normal") ? 1 : 0;
        bellData.deviceMessage = jsonMsg.msg;
        bellData.deviceAvailable = 1;
        bellData.deviceEvent = 1; // deviceEvent = 1 = event case
        bellData.replyEvent = JSON.stringify({ "user": null, "replyStatus": "unread" });
        // create local database
        var message = await DeviceBellController.createDeviceBell(bellData);
        console.log(message);
      }
    }
  } else if (deviceType === 'gunther') {
    var guntherData = { "eventId": uuid.v4() };
    guntherData.deviceId = jsonMsg.clientId;
    const isDeviceSub = await DeviceController.findDeviceCode(guntherData.deviceId);
    if (isDeviceSub) {
      if (jsonMsg.msg === 'fall') {
        guntherData.deviceBattery = (jsonMsg.battery === "normal") ? 1 : 0;
        guntherData.deviceAvailable = 1;
        guntherData.deviceEvent = 1; // deviceEvent = 1 = event case
        guntherData.deviceMessage = jsonMsg.msg;
        guntherData.replyEvent = JSON.stringify({ "user": null, "replyStatus": "unread" });
        // create local database
        var message = await DeviceGuntherController.createDeviceGunther(guntherData);
        console.log(message);
      } else if (jsonMsg.msg === 'start' || jsonMsg === 'moving' || jsonMsg === 'not_moving') {
        guntherData.deviceBattery = (jsonMsg.battery === "normal") ? 1 : 0;
        guntherData.deviceAvailable = 1;
        guntherData.deviceEvent = 0; // deviceEvent = 1 = event case
        guntherData.deviceMessage = jsonMsg.msg;
        guntherData.replyEvent = null;
        // create local database
        var message = await DeviceBellController.createDeviceBell(guntherData);
        console.log(message);
      }
    }
  }
}

async function replyLocalData(deviceId, deviceType, replyJson) {
  if (deviceType === 'bell') {
    var bellData = { "eventId": uuid.v4() };
    bellData.deviceId = deviceId;
    bellData.replyEvent = JSON.stringify(replyJson);
    bellData.deviceEvent = 2; // deviceEvent = 2 = reply case
    bellData.deviceAvailable = null;
    bellData.deviceBattery = null;
    // create local database
    var message = await DeviceBellController.createDeviceBell(bellData);
    console.log(message);
  } else if (deviceType === 'gunther') {
    var guntherData = { "eventId": uuid.v4() };
    guntherData.deviceId = deviceId;
    guntherData.replyEvent = JSON.stringify(replyJson);
    guntherData.deviceEvent = 2; // deviceEvent = 2 = reply case
    guntherData.deviceAvailable = null;
    guntherData.deviceBattery = null;
    // create local database
    var message = await DeviceGuntherController.createDeviceGunther(guntherData);
    console.log(message);
  }
}

module.exports = { zigbeeLocalData, espLocalData, replyLocalData };

// async function writeBellData(deviceId, message)  {
//   // A post entry.
//   var bellData = {
//     msg: message, // message 0 = ring, 1 = call
//     timestamp: moment().utc().format('YYYY-MM-DD HH:mm:ss'), // time
//     deviceId: deviceId, //token
//   };

//   var result = await postToServer(configFile.hostname, configFile.BELLEVENTS_API, JSON.stringify(bellData), true);
//   console.log(result);
// }

// async function writeGateData(deviceId, message)  {
//   var gateData = {
//     msg: message, // message 0 = warning
//     deviceId: deviceId, //token
//   };
//   var result = await postToServer(configFile.hostname, configFile.GATEEVENTS_API, JSON.stringify(gateData), true);
//   console.log(result);
// }

// async function writeGuntherData(deviceId, message)  {
//   var guntherData = {
//     msg: message, // message 0 = falling
//     deviceId: deviceId, //token
//   };

//   var result = await postToServer(configFile.hostname, configFile.GUNTHEREVENTS_API, JSON.stringify(guntherData), true);
//   console.log(result);

// }
