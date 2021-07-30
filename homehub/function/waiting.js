const firebase = require('firebase-admin');
// import controller
const DeviceController = require('../controller/deviceController');

// import writing function for creating device reply event
const writing = require('./writing');
// import find function for finding device hitory event
const find = require('./find'); 
// import play sound
//const play = require('./play')

// set variables
const databaseURL = 'https://connectedpeaceofmind-817d4.firebaseio.com/';
const serviceAccount = require('../config/connectedpeaceofmind-817d4-firebase-adminsdk-38alx-fcd18fc0f0.json');
const deviceController = require('../controller/deviceController');

// setting firebase
var configFirebase = {
  credential: firebase.credential.cert(serviceAccount),
  databaseURL: databaseURL
};
firebase.initializeApp(configFirebase);
var database = firebase.database();

function waitForUserReplied(configFile, client) {
  var userRepliedRef = database.ref('/replied/' + configFile.uuid);
  userRepliedRef.off();
  console.log("wait for user replied "+ userRepliedRef);
  userRepliedRef.on("value", function(snapshot) {
    console.log(snapshot.val());
    if(snapshot.val() != null) {
      let value = snapshot.val();
      snapshot.forEach(async function(data) {
        let deviceId = value[data.key].deviceId;
        let deviceType = value[data.key].deviceType;
        let userId = data.key;
        let userName = value[data.key].userName;
        let linkSound = value[data.key].link;
        database.ref('/replied/' + configFile.uuid + '/' + userId).remove();
        var replyData = {"user": userName.toLowerCase(), "replyStatus": "replied"};
        await writing.replyLocalData(deviceId, deviceType, replyData);
        // play sound
        //if (linkSound != null) {play.playSound(linkSound);}
        //get client id from bell
        var replyMsg = {"clientId": deviceId, "msg": "success", "device": userName.toLowerCase()};
        client.publish('homehub', JSON.stringify(replyMsg));
      });
    }
  });
}

function waitForUserRead(configFile, client) {
  var userReadRef = database.ref('/read/' + configFile.uuid);
  userReadRef.off();
  console.log("wait for user read "+ userReadRef);
  userReadRef.on("value", function(snapshot) {
    console.log(snapshot.val());
    if(snapshot.val() != null) {
      let value = snapshot.val();
      snapshot.forEach(async function(data) {
        let deviceId = value[data.key].deviceId;
        let deviceType = value[data.key].deviceType;
        let userId = data.key;
        let userName = data.userName;
        database.ref('/read/' + configFile.uuid + '/' + userId).remove();
        var replyData = {"user": userName.toLowerCase(), "replyStatus": "read"};
        await writing.replyLocalData(deviceId, deviceType, replyData);
        //get client id from bell
        var replyMsg = {"clientId": deviceId, "msg": "success", "device": userName.toLowerCase()};
        client.publish('homehub', JSON.stringify(replyMsg));
      });
    }
  });
}

function waitForRequestHistory(configFile) {
  var historyRef = database.ref('/requestHistory/' + configFile.uuid);
  historyRef.off();
  console.log("wait for requesting history " + historyRef);
  historyRef.on("value", function(snapshot) {
    console.log(snapshot.val());
    if(snapshot.val() != null) {
      let value = snapshot.val();
      snapshot.forEach(async function(data) {
        let userId = value[data.key].userId;
        let deviceType = value[data.key].deviceType;
        let deviceId = data.key;
        database.ref('/requestHistory/' + configFile.uuid + '/' + deviceId).remove();
        console.log('user id: ' + userId + ' id: ' + deviceId + ' type: ' + deviceType);
        const dataEvents = await find.findDeviceHistory(deviceId, deviceType);
        if (dataEvents) {
          var count = 0;
          dataEvents.forEach( function(dataEvent) {
            count++;
            var sendHistoryRef = database.ref('sendHistory/');
            var deviceRef = sendHistoryRef.child(userId + '/' + deviceId + '/' + dataEvent.eventId);
            deviceRef.set({deviceEvent: dataEvent.deviceEvent, replyEvent: dataEvent.replyEvent, created: dataEvent.created});
            if (count == dataEvents.length) {
              obj = { "userId": userId, "deviceId": deviceId}
              deviceController.updateHistorytoServer(configFile.hostname, configFile.DEVICEHISTORY_API, configFile.headers, obj)
            }
          })
        } else {
          obj = { "userId": userId, "deviceId": deviceId}
          deviceController.updateHistorytoServer(configFile.hostname, configFile.DEVICEHISTORY_API, configFile.headers, obj)
        }
      });
    }
  });
}
  
function waitForAddDevice(configFile) {
  var deviceRef = database.ref('/addDevice/' + configFile.uuid);
  deviceRef.off();
  deviceRef.on("value", function(snapshot) {
    console.log(snapshot.val());
    if(snapshot.val() != null) {
      let value = snapshot.val();
      snapshot.forEach( async function(data) {
        let type = value[data.key].type;
        let id = data.key;
        database.ref('/addDevice/' + configFile.uuid + '/' + id).remove();
        console.log('request ' + type + ' id: ' + id);
        var obj = {"deviceId": id, "deviceType": type}
        const isSuccess = await DeviceController.createDeviceCode(obj);
        console.log(isSuccess)
      });
    }
  });
}

module.exports = {
  waitForUserReplied,
  waitForUserRead,
  waitForAddDevice,
  waitForRequestHistory,
};