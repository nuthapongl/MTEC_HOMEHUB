function increaseTime(deviceId) {
    count = count + 1;
    console.log(count);
    if (count == 30) {
      writeGateData(deviceId, 0);
    } else if (count == 60) {
      writeGateData(deviceId, 0);
    }
  }