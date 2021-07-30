const db = require('../config/dbconfig');
const axios = require('axios');

const sequelize = db.sequelize;
const DeviceSwitch = db.deviceSwitch;

async function createDeviceSwitch(obj) {
    const deviceData = {};
    try { 
        deviceData.eventId = obj.eventId;
        deviceData.deviceId = obj.deviceId;
        deviceData.deviceBattery = (obj.battery > 20) ? 1:0;
        deviceData.deviceClick = obj.click;
        deviceData.created = new Date();
        deviceData.updated = new Date();
        deviceData.createdBy = 'system';
        deviceData.updatedBy = 'system';
        let newDevice = null;
        if (deviceData) {
            newDevice = await sequelize.transaction(function(t) {
                return DeviceSwitch.create(deviceData, { transaction: t });
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

async function currentDeviceSwitch(deviceId) {
    const queryDeviceSwitchs = {deviceId: deviceId};
    try {
        const deviceSwitchs = await DeviceSwitch.findAll({
            where: queryDeviceSwitchs,
        })
        if (deviceSwitchs.length != 0) {
            return deviceSwitchs[deviceSwitchs.length - 1];
        } else {
            return null
        }
    } catch(error) {
        console.error(error.message)
        return null;
    }
}

async function updateDeviceSwitchToServer(hostname, api, headers, obj) {
    var deviceStatusData = {
        deviceBattery: obj.deviceBattery, 
        deviceAvailable: 1,
        deviceDetail: JSON.stringify({
            battery: obj.deviceBattery,
            click: obj.deviceClick,
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

module.exports = {createDeviceSwitch, currentDeviceSwitch, updateDeviceSwitchToServer};