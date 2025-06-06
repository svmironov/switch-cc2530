const fz = require('zigbee-herdsman-converters/converters/fromZigbee');
const exposes = require('zigbee-herdsman-converters/lib/exposes');
const e = exposes.presets;

const customCommandOn = {
    cluster: 'genOnOff',
    type: 'commandOn',
    convert: (model, msg, publish, options, meta) => {
        return {action: `on_${msg.endpoint.ID}`};
    },
};

const customCommandOff = {
    cluster: 'genOnOff',
    type: 'commandOff',
    convert: (model, msg, publish, options, meta) => {
        return {action: `off_${msg.endpoint.ID}`};
    },
};

const customLevelMove = {
    cluster: 'genLevelCtrl',
    type: 'commandMove',
    convert: (model, msg, publish, options, meta) => {
        console.log("Received commandMove:", JSON.stringify(msg, null, 2));

        const moveMode = msg.data.moveMode;
        let direction;

        if (moveMode === 0) {
            direction = 'move_up';
        } else if (moveMode === 1) {
            direction = 'move_down';
        } else {
            direction = 'move_unknown';
        }

        return {action: `${direction}_${msg.endpoint.ID}`};
    },
};

const customLevelStop = {
    cluster: 'genLevelCtrl',
    type: 'commandStop',
    convert: (model, msg, publish, options, meta) => {
        console.log("Received commandStop:", JSON.stringify(msg, null, 2));
        return {action: `stop_${msg.endpoint.ID}`};
    },
};

const customMoveToLevel = {
    cluster: 'genLevelCtrl',
    type: 'commandMoveToLevelWithOnOff',
    convert: (model, msg, publish, options, meta) => {
        console.log("Received commandMoveToLevelWithOnOff:", JSON.stringify(msg, null, 2));

        const level = msg.data.level;
        return {action: `brightness_${msg.endpoint.ID}`, brightness: level};
    },
};

const definition = {
    zigbeeModel: ['One'],
    model: 'One',
    vendor: 'MagicSwitch',
    icon: 'device_icons/One.png',
    description: 'Switch with dual endpoint and dimmer support',
    fromZigbee: [
        customCommandOn,
        customCommandOff,
        customMoveToLevel,
        customLevelMove,
        customLevelStop,
    ],
    toZigbee: [],
    exposes: [
        e.action([
            'on_1', 'off_1', 'on_2', 'off_2',
            'brightness_1', 'brightness_2',
            'move_up_1', 'move_down_1', 'move_unknown_1', 'stop_1',
            'move_up_2', 'move_down_2', 'move_unknown_2', 'stop_2',
        ]),
    ],
};

module.exports = [definition];