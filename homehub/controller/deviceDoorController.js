const db = require('../config/dbconfig');
const axios = require('axios');

const sequelize = db.sequelize;
const DeviceDoor = db.deviceDoor;

async function createDeviceDoor(obj) {
    const deviceData = {};
    try { 
        deviceData.eventId = obj.eventId;
        deviceData.deviceId = obj.deviceId;
        deviceData.deviceBattery = (obj.battery > 20) ? 1:0;
        deviceData.deviceContact = obj.contact ? 1: 0;
        deviceData.created = new Date();
        deviceData.updated = new Date();
        deviceData.createdBy = 'system';
        deviceData.updatedBy = 'system';
        let newDevice = null;
        if (deviceData) {
            newDevice = await sequelize.transaction(function(t) {
                return DeviceDoor.create(deviceData, { transaction: t });
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

async function currentDeviceDoor(deviceId) {
    const queryDeviceDoors = {deviceId: deviceId};
    try {
        const deviceDoors = await DeviceDoor.findAll({
            where: queryDeviceDoors,
        })
        if (deviceDoors.length != 0) {
            return deviceDoors[deviceDoors.length - 1];
        } else {
            return null
        }
    } catch(error) {
        console.error(error.message)
        return null;
    }
}

async function updateDeviceDoorToServer(hostname, api, headers, obj) {
    var deviceStatusData = {
        deviceBattery: obj.deviceBattery, 
        deviceAvailable: 1,
        deviceDetail: JSON.stringify({
            battery: obj.deviceBattery,
            contact: obj.deviceContact,
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

module.exports = {createDeviceDoor, currentDeviceDoor, updateDeviceDoorToServer};