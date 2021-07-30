const db = require('../config/dbconfig');
const axios = require('axios');

const sequelize = db.sequelize;
const DeviceOccupancy = db.deviceOccupancy;

async function createDeviceOccupancy(obj) {
    const deviceData = {};
    try { 
        deviceData.eventId = obj.eventId;
        deviceData.deviceId = obj.deviceId;
        deviceData.deviceBattery = (obj.battery > 20) ? 1:0;
        deviceData.deviceIlluminance = obj.illuminance;
        deviceData.deviceIlluminanceLux = obj.illuminance_lux;
        deviceData.deviceOccupancy = obj.occupancy ? 1 : 0;
        deviceData.created = new Date();
        deviceData.updated = new Date();
        deviceData.createdBy = 'system';
        deviceData.updatedBy = 'system';
        let newDevice = null;
        if (deviceData) {
            newDevice = await sequelize.transaction(function(t) {
                return DeviceOccupancy.create(deviceData, { transaction: t });
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

async function currentDeviceOccupancy(deviceId) {
    const queryDeviceOccupancy = {deviceId: deviceId};
    try {
        const deviceOccupancys = await DeviceOccupancy.findAll({
            where: queryDeviceOccupancy,
        })
        if (deviceOccupancys.length != 0) {
            return deviceOccupancys[deviceOccupancys.length - 1];
        } else {
            return null
        }
    } catch(error) {
        console.error(error.message)
        return null;
    }
}

async function updateDeviceOccupancyToServer(hostname, api, headers, obj) {
    var deviceStatusData = {
        deviceBattery: obj.deviceBattery,
        deviceAvailable: 1,
        deviceDetail: JSON.stringify({
            illuminance: obj.deviceIlluminance,
            illuminance_lux: obj.deviceIlluminanceLux,
            occupancy: obj.deviceOccupancy,
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

module.exports = {createDeviceOccupancy, currentDeviceOccupancy, updateDeviceOccupancyToServer};