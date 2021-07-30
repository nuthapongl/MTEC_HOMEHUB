module.exports = (sequelize, Sequelize) => {
    const Item = sequelize.define(
        'device_gunther',
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
            deviceAvailable: {
                type: Sequelize.INTEGER,
                field: 'device_available'
            },
            deviceMessage: {
                type: Sequelize.STRING,
                field: 'device_message'
            },
            deviceEvent: {
                type: Sequelize.INTEGER,
                field: 'device_event'
            },
            replyEvent: {
                type: Sequelize.TEXT,
                field: 'reply_event'
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
            tableName: 'table_device_gunther',
        }
    );
    return Item;
};