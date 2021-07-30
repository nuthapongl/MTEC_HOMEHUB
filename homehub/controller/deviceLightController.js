const db = require('../config/dbconfig');
const axios = require('axios');

const sequelize = db.sequelize;
const DeviceLight = db.deviceLight;
DeviceLight.sync()

async function createDeviceLight(obj) {
    const deviceData = {};
    try { 
        deviceData.eventId = obj.eventId;
        deviceData.deviceId = obj.deviceId;
        deviceData.deviceState = obj.state;
        deviceData.deviceBrightness = obj.brightness;
        deviceData.deviceColorTemp = obj.color_temp;
        deviceData.created = new Date();
        deviceData.updated = new Date();
        deviceData.createdBy = 'system';
        deviceData.updatedBy = 'system';
        let newDevice = null;
        if (deviceData) {
            newDevice = await sequelize.transaction(function(t) {
                return DeviceLight.create(deviceData, { transaction: t });
            });
            if (newDevice != null) {
                return true;
            } else {
                return false;
            }
        }
    } catch(error) {
        console.error(error.message)
        return false;
    }
}

async function currentDeviceLight(deviceId) {
    const queryDeviceLights = {deviceId: deviceId};
    try {
        const deviceLights = await DeviceLight.findAll({
            where: queryDeviceLights,
        })
        if (deviceLights.length != 0) {
            return deviceLights[deviceLights.length - 1];
        } else {
            return null
        }
    } catch(error) {
        console.error(error.message)
        return null
    }
}

async function updateDeviceLightToServer(hostname, api, headers, obj) {
    var deviceStatusData = {
        deviceBattery: null, // message 0 = falling
        deviceAvailable: 1,
        deviceDetail: JSON.stringify({
            state: obj.deviceState,
            brightness: obj.deviceBrightness,
            colortemp: obj.deviceColorTemp,
        }),
        deviceId: obj.deviceId,
    };
    response = await axios.post(hostname + api, JSON.stringify(deviceStatusData), {headers: headers});
    if(response) {
        return response.data;
    } else {
        return 'cannot reach host';
    }
}

module.exports = {createDeviceLight, currentDeviceLight, updateDeviceLightToServer};