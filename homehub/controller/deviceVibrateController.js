const db = require('../config/dbconfig');
const axios = require('axios');

const sequelize = db.sequelize;
const DeviceVibrate = db.deviceVibrate;

async function createDeviceVibrate(obj) {
    const deviceData = {};
    try { 
        deviceData.eventId = obj.eventId;
        deviceData.deviceId = obj.deviceId;
        deviceData.deviceBattery = (obj.battery > 20) ? 1:0;
        deviceData.deviceAction = obj.action;
        deviceData.deviceAngleX = obj.angle_x;
        deviceData.deviceAngleY = obj.angle_y;
        deviceData.deviceAngleZ = obj.angle_z;
        deviceData.created = new Date();
        deviceData.updated = new Date();
        deviceData.createdBy = 'system';
        deviceData.updatedBy = 'system';
        let newDevice = null;
        if (deviceData) {
            newDevice = await sequelize.transaction(function(t) {
                return DeviceVibrate.create(deviceData, { transaction: t });
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

async function currentDeviceVibrate(deviceId) {
    const queryDeviceVibrates = {deviceId: deviceId};
    try {
        const deviceVibrates = await DeviceVibrate.findAll({
            where: queryDeviceVibrates,
        })
        if (deviceVibrates.length != 0) {
            return deviceVibrates[deviceVibrates.length - 1];
        } else {
            return null
        }
    } catch(error) {
        console.error(error.message)
        return null;
    }
}

async function updateDeviceVibrateToServer(hostname, api, headers, obj) {
    var deviceStatusData = {
        deviceBattery: obj.deviceBattery,
        deviceAvailable: 1,
        deviceDetail: JSON.stringify({
            action: obj.deviceAction,
            anglex: obj.deviceAngleX,
            angley: obj.deviceAngleY,
            anglez: obj.deviceAngleZ
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

module.exports = {createDeviceVibrate, currentDeviceVibrate, updateDeviceVibrateToServer};