const Sequelize = require('sequelize');
const env = require('./env');
const sequelize = new Sequelize({
  dialect: env.dialect,
  storage: env.storage,
  pool: {
    max: env.max,
    min: env.pool.min,
    acquire: env.pool.acquire,
    idle: env.pool.idle
  }
});

sequelize.authenticate().then(() => {
    console.log('Connection has been established successfully.');
}).catch(err => {
    console.error('Unable to connect to the database:', err);
});

const db = {};
db.Sequelize = Sequelize;
db.sequelize = sequelize;

//import model
db.deviceOccupancy = require('../model/deviceOccupancy.js')(sequelize, Sequelize);
db.deviceLight = require('../model/deviceLight.js')(sequelize, Sequelize);
db.deviceVibrate = require('../model/deviceVibrate.js')(sequelize, Sequelize);
db.deviceDoor = require('../model/deviceDoor.js')(sequelize, Sequelize);
db.deviceCode = require('../model/deviceCode.js')(sequelize, Sequelize);
db.deviceBell = require('../model/deviceBell.js')(sequelize, Sequelize);
db.deviceGunther = require('../model/deviceGunther.js')(sequelize, Sequelize);
db.deviceSwitch = require('../model/deviceSwitch.js')(sequelize, Sequelize);
// sync database
db.deviceOccupancy.sync();
db.deviceLight.sync();
db.deviceVibrate.sync();
db.deviceDoor.sync();
db.deviceCode.sync();
db.deviceBell.sync();
db.deviceGunther.sync();

module.exports = db;