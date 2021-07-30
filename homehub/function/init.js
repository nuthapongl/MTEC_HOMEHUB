const fs = require('fs');
const axios = require('axios');

function readFile(fileName) {
  return new Promise(function (resolve, reject) {
    fs.readFile(fileName, 'utf8', function (error, data) {
      if (error) {
        return reject(error)
      } else {
        resolve(data);
      }
    })
  });
}

function settingConfig(data) {
  return new Promise(async function(resolve, reject) {
    if(data) {
      var configFile = JSON.parse(data);
      var serialNo = {
        serial: configFile.uuid,
        device: 'homehub'
      }
      var response = await axios.post(configFile.hostname + configFile.LOGIN_API, serialNo);
      configFile.jwtToken = response.data.token;
      configFile.headers = {
        'Content-Type': 'application/json',
        'Authorization': 'Bearer '+ configFile.jwtToken,
      }
    }
    resolve(configFile);
  })
}

module.exports = {readFile, settingConfig};