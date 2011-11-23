# nodewm
*Is a FAST nodejs library for signing WebMoney requests written in C/C++*

## Install:
1) go to the directory with nodewm library

2) execute `node-waf configure build`

3) get module from `./build/default/wmsigner.node`

## Using nodewm

var wmsigner = require("./build/default/wmsigner");

var sign = wmsigner.sign('Your Wallet', 'Your Password', 'Your Base 64 Encoded KWM file', 'String to sign');

console.log('SIGN: ' + sign);
