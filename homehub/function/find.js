// controll database of device
const DeviceBellController = require('../controller/deviceBellController');
const DeviceGuntherController = require('../controller/deviceGuntherController');

async function findDeviceHistory(deviceId, deviceType) {
    var dataEvents
    if (deviceType === 'bell') {
        dataEvents = await DeviceBellController.historyDeviceEventBell(deviceId);
    } else if (deviceType === 'gunther') {
        dataEvents = await DeviceGuntherController.historyDeviceEventBell(deviceId);
    }
    return dataEvents;
}

module.exports = {
    findDeviceHistory,
  };