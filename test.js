var wmsigner = require("./build/default/wmsigner");
var sign = wmsigner.sign('<Your Wallet>', 'Your Password', 'Your Base64 encoded KWM file', 'string to sign');
console.log('SIGN: ' + sign);
