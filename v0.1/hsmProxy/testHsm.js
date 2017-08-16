// testHsm.js
const hsm = require('./build/Release/hsmproxy');
console.log(hsm);

var userPin="1234";
console.log("hsm.init("+userPin+")="+hsm.init(userPin));
