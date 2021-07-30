module.exports = (sequelize, Sequelize) => {
    const Item = sequelize.define(
        'device_code',
        {
            deviceId: {
                type: Sequelize.STRING,
                field: 'device_id',
                primaryKey: true
            },
            deviceType: {
                type: Sequelize.STRING,
                field: 'device_type',
            },
            created: {
                type: 'TIMESTAMP',
                field: 'created'
            },
            createdBy: {
                type: Sequelize.STRING,
                field: 'created_by'
            }
        },
        {
            timestamps: false,
            freezeTableName: true,
            tableName: 'table_device_code',
        }
    );
    return Item;
};