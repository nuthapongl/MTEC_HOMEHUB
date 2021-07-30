const db = require('../config/dbconfig');
const axios = require('axios');
const Sequelize = require('sequelize');

const sequelize = db.sequelize;
const DeviceBell = db.deviceBell;
const Op = Sequelize.Op;

async function createDeviceBell(obj) {
    const deviceData = {};
    try { 
        deviceData.eventId = obj.eventId;
        deviceData.deviceId = obj.deviceId;
        deviceData.deviceBattery = obj.deviceBattery;
        deviceData.deviceAvailable = obj.deviceBattery;
        deviceData.deviceMessage = obj.deviceMessage;
        deviceData.deviceEvent = obj.deviceEvent;
        deviceData.replyEvent = obj.replyEvent;
        deviceData.created = new Date();
        deviceData.updated = new Date();
        deviceData.createdBy = 'system';
        deviceData.updatedBy = 'system';
        let newDevice = null;
        if (deviceData) {
            newDevice = await sequelize.transaction(function(t) {
                return DeviceBell.create(deviceData, { transaction: t });
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

async function currentDeviceBell(deviceId) {
    const queryDeviceBells = {deviceId: deviceId};
    try {
        const deviceBells = await DeviceBell.findAll({
            where: queryDeviceBells,
        })
        if (deviceBells.length != 0) {
            return deviceBells[deviceBells.length - 1];
        } else {
            return null
        }
    } catch(error) {
        console.error(error.message)
        return null;
    }
}

async function currentDeviceEventBell(deviceId) {
    const queryDeviceBells = {deviceId: deviceId, deviceEvent: 1};
    try {
        const deviceBells = await DeviceBell.findAll({
            where: queryDeviceBells,
        })
        if (deviceBells.length != 0) {
            return deviceBells[deviceBells.length - 1];
        } else {
            return null
        }
    } catch(error) {
        console.error(error.message)
        return null;
    }
}

async function historyDeviceEventBell(deviceId) {
    const queryDeviceBells = {deviceId: deviceId, deviceEvent: {[Op.or]: [1,2]}};
    try {
        const deviceBells = await DeviceBell.findAll({
            where: queryDeviceBells,
        })
        if (deviceBells.length != 0) {
            return deviceBells;
        } else {
            return null
        }
    } catch(error) {
        console.error(error.message)
        return null;
    }
}

async function updateDeviceBellToServer(hostname, api, headers, obj) {
    var deviceStatusData = {
        deviceBattery: obj.deviceBattery,
        deviceAvailable: obj.deviceAvailable,
        deviceId: obj.deviceId,
    };
    response = await axios.post(hostname + api, JSON.stringify(deviceStatusData), {headers: headers});
    if(response) {
        return response.data;
    } else {
        return 'cannot reach host';
    }
}

async function updateDeviceEventBellToServer(hostname, api, headers, obj) {
    var deviceEventData = {
        deviceEvent: obj.deviceEvent,
        deviceId: obj.deviceId,
    }
    response = await axios.post(hostname + api, JSON.stringify(deviceEventData), {headers: headers});
    if(response) {
        return response.data;
    } else {
        return 'cannot reach host';
    }
}

module.exports = {
    createDeviceBell,
    currentDeviceBell,
    currentDeviceEventBell,
    updateDeviceBellToServer,
    updateDeviceEventBellToServer,
    historyDeviceEventBell,
};