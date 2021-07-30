const DeviceOccupancyController = require('../controller/deviceOccupancyController');
const DeviceLightController = require('../controller/deviceLightController');
const DeviceDoorController = require('../controller/deviceDoorController');
const DeviceVibrateController = require('../controller/deviceVibrateController');
const DeviceBellController = require('../controller/deviceBellController');
const DeviceGuntherController = require('../controller/deviceGuntherController');
const DeviceSwitchController = require('../controller/deviceSwitchController');
const DeviceController = require('../controller/deviceController');

async function deviceDatatoServer(configFile) {
    const deviceCodes = await DeviceController.findAllDeviceCodes();
    if (deviceCodes != null) {
        for(i = 0; i < deviceCodes.length; i++) {
            if (deviceCodes[i].deviceType == "occupancy") {
                const currentOccupancyData = await DeviceOccupancyController.currentDeviceOccupancy(deviceCodes[i].deviceId);
                if (currentOccupancyData) {
                    const result = await DeviceOccupancyController.updateDeviceOccupancyToServer(configFile.hostname, configFile.DEVICESTATUS_API, configFile.headers, currentOccupancyData);
                    console.log(result);
                }
            } else if (deviceCodes[i].deviceType == "door") {
                const currentDoorData = await DeviceDoorController.currentDeviceDoor(deviceCodes[i].deviceId);
                if (currentDoorData) {
                    const result = await DeviceDoorController.updateDeviceDoorToServer(configFile.hostname, configFile.DEVICESTATUS_API, configFile.headers, currentDoorData);
                    console.log(result);
                }
            } else if (deviceCodes[i].deviceType == "light") {
                const currentLightData = await DeviceLightController.currentDeviceLight(deviceCodes[i].deviceId);
                if (currentLightData) {
                    const result = await DeviceLightController.updateDeviceLightToServer(configFile.hostname, configFile.DEVICESTATUS_API, configFile.headers, currentLightData);
                    console.log(result);
                }
            } else if (deviceCodes[i].deviceType == "vibrate") {
                const currentVibrateData = await DeviceVibrateController.currentDeviceVibrate(deviceCodes[i].deviceId);
                if (currentVibrateData) {
                    const result = await DeviceVibrateController.updateDeviceVibrateToServer(configFile.hostname, configFile.DEVICESTATUS_API, configFile.headers, currentVibrateData);
                    console.log(result);
                }
            } else if (deviceCodes[i].deviceType == "switch") {
                const currentSwitchData = await DeviceSwitchController.currentDeviceSwitch(deviceCodes[i].deviceId);
                if (currentSwitchData) {
                    const result = await DeviceSwitchController.updateDeviceSwitchToServer(configFile.hostname, configFile.DEVICESTATUS_API, configFile.headers, currentVibrateData);
                    console.log(result);
                }
            } else if (deviceCodes[i].deviceType == "bell") {
                const currentBellData = await DeviceBellController.currentDeviceBell(deviceCodes[i].deviceId);
                if (currentBellData) {
                    const result = await DeviceBellController.updateDeviceBellToServer(configFile.hostname, configFile.DEVICESTATUS_API, configFile.headers, currentBellData);
                    console.log(result);
                }
            } else if (deviceCodes[i].deviceType == "gunther") {
                const currentGuntherData = await DeviceGuntherController.currentDeviceGunther(deviceCodes[i].deviceId);
                if (currentGuntherData) {
                    const result = await DeviceGuntherController.updateDeviceGuntherToServer(configFile.hostname, configFile.DEVICESTATUS_API, configFile.headers, currentBellData);
                    console.log(result);
                }
            }
        }
    }
}

async function deviceEventtoServerbyId(configFile, deviceId) {
    const currentBellData = await DeviceBellController.currentDeviceEventBell(deviceId);
    if (currentBellData) {
        const result = await DeviceBellController.updateDeviceEventBellToServer(configFile.hostname, configFile.DEVICEEVENT_API, configFile.headers, currentBellData);
        console.log(result);
    }
}

module.exports = {deviceDatatoServer, deviceEventtoServerbyId};