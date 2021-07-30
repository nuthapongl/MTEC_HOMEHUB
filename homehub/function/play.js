const mpg = require('mpg123');
const player = new mpg.MpgPlayer();

function playSound(linkSound) {
    player.play(linkSound);
}

module.exports = {playSound}