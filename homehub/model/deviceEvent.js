module.exports = (sequelize, Sequelize) => {
    const Item = sequelize.define(
        'device_event',
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
            deviceStatus: {
                type: Sequelize.INTEGER,
                field: 'device_status'
            },
            replyDetail: {
                type: Sequelize.TEXT,
                field: 'reply_detail'
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
            tableName: 'table_device_event',
        }
    );
    return Item;
};