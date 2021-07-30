module.exports = (sequelize, Sequelize) => {
    const Item = sequelize.define(
        'device_vibrate',
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
            deviceAction: {
                type: Sequelize.STRING,
                field: 'device_action'
            },
            deviceAngleX: {
                type: Sequelize.INTEGER,
                field: 'device_angle_x'
            },
            deviceAngleY: {
                type: Sequelize.INTEGER,
                field: 'device_angle_y'
            },
            deviceAngleZ: {
                type: Sequelize.INTEGER,
                field: 'device_angle_z'
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
            tableName: 'table_device_vibrate',
        }
    );
    return Item;
};