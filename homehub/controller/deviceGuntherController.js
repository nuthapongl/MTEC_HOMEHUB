const db = require('../config/dbconfig');
const axios = require('axios');
const Sequelize = require('sequelize');

const sequelize = db.sequelize;
const DeviceGunther = db.deviceGunther;
const Op = Sequelize.Op;

async function createDeviceGunther(obj) {
    const deviceData = {};
    try { 
        deviceData.eventId = obj.eventId;
        deviceData.deviceId = obj.deviceId;
        deviceData.deviceBattery = obj.deviceBattery;
        deviceData.deviceAvailable = obj.deviceAvailable;
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
                return DeviceGunther.create(deviceData, { transaction: t });
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

async function currentDeviceGunther(deviceId) {
    const queryDeviceGunthers = {deviceId: deviceId};
    try {
        const deviceGunthers = await DeviceGunther.findAll({
            where: queryDeviceGunthers,
        })
        if (deviceGunthers.length != 0) {
            return deviceGunthers[deviceGunthers.length - 1];
        } else {
            return null
        }
    } catch(error) {
        console.error(error.message)
        return null;
    }
}

async function currentDeviceEventGunther(deviceId) {
    const queryDeviceGunthers = {deviceId: deviceId, deviceEvent: 1};
    try {
        const deviceGunthers = await DeviceGunther.findAll({
            where: queryDeviceGunthers,
        })
        if (deviceGunthers.length != 0) {
            return deviceGunthers[deviceGunthers.length - 1];
        } else {
            return null
        }
    } catch(error) {
        console.error(error.message)
        return null;
    }
}

async function historyDeviceEventGunther(deviceId) {
    const queryDeviceGunthers = {deviceId: deviceId, deviceEvent: {[Op.or]: [1,2]}};
    try {
        const deviceGunthers = await DeviceGunther.findAll({
            where: queryDeviceGunthers,
        })
        if (deviceGunthers.length != 0) {
            return deviceGunthers;
        } else {
            return null
        }
    } catch(error) {
        console.error(error.message)
        return null;
    }
}

async function updateDeviceGuntherToServer(hostname, api, headers, obj) {
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

async function updateDeviceEventGuntherToServer(hostname, api, headers, obj) {
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
    createDeviceGunther,
    currentDeviceGunther,
    currentDeviceEventGunther,
    updateDeviceGuntherToServer,
    updateDeviceEventGuntherToServer,
    historyDeviceEventGunther,
};