const db = require('../config/dbconfig');

const sequelize = db.sequelize;
const DeviceCode = db.deviceCode;

async function createDeviceCode(obj) {
    const deviceData = {};
    try { 
        deviceData.deviceId = obj.deviceId;
        deviceData.deviceType = obj.deviceType;
        deviceData.created = new Date();
        deviceData.createdBy = 'system';
        let newDevice = null;
        if (deviceData) {
            newDevice = await sequelize.transaction(function(t) {
                return DeviceCode.create(deviceData, { transaction: t });
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

async function findAllDeviceCodes() {
    try {
        const deviceCodes = await DeviceCode.findAll();
        if (deviceCodes.length != 0) {
            return deviceCodes;
        }
        else {
            return null;
        }
    } catch(error) {
        console.error(error.message)
        return null;
    }
}

async function findDeviceCode(deviceId) {
    const queryDeviceCodes = {deviceId: deviceId};
    try {
        const deviceCodes = await DeviceCode.findAll({
            where: queryDeviceCodes,
        })
        if (deviceCodes.length != 0) {
            return true;
        }
        else {
            return false;
        }
    } catch(error) {
        console.error(error.message)
        return false;
    }
}


module.exports = {createDeviceCode, findAllDeviceCodes, findDeviceCode};