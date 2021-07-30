module.exports = (sequelize, Sequelize) => {
    const Item = sequelize.define(
        'device_light',
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
            deviceState: {
                type: Sequelize.STRING,
                field: 'device_state'
            },
            deviceBrightness: {
                type: Sequelize.INTEGER,
                field: 'device_brightness'
            },
            deviceColorTemp: {
                type: Sequelize.INTEGER,
                field: 'device_colortemp'
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
            tableName: 'table_device_light',
        }
    );
    return Item;
};