module.exports = (sequelize, Sequelize) => {
    const Item = sequelize.define(
        'device_occupancy',
        {
            eventId: {
                type: Sequelize.STRING,
                field: 'event_id',
                primaryKey: true
            },
            deviceId: {
                type: Sequelize.STRING,
                field: 'device_id',
                primaryKey: true
            },
            deviceBattery: {
                type: Sequelize.INTEGER,
                field: 'device_battery'
            },
            deviceIlluminance: {
                type: Sequelize.INTEGER,
                field: 'device_illuminance'
            },
            deviceIlluminanceLux: {
                type: Sequelize.INTEGER,
                field: 'device_illuminance_lux'
            },
            deviceOccupancy: {
                type: Sequelize.INTEGER,
                field: 'device_occupancy'
            },
            created: {
                type: 'TIMESTAMP',
                field: 'created'
            },
            updated: {
                type: 'TIMESTAMP',
                field: 'updated'
            },
            createdBy: {
                type: Sequelize.STRING,
                field: 'created_by'
            },
            updatedBy: {
                type: Sequelize.STRING,
                field: 'updated_by'
            }
        },
        {
            timestamps: false,
            freezeTableName: true,
            tableName: 'table_device_occupancy',
        }
    );
    return Item;
};