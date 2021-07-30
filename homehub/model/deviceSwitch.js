module.exports = (sequelize, Sequelize) => {
    const Item = sequelize.define(
        'device_switch',
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
            deviceClick: {
                type: Sequelize.STRING,
                field: 'device_click'
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
            tableName: 'table_device_switch',
        }
    );
    return Item;
};