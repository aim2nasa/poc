// testHsm.js
const hsm = require('./build/Release/hsmproxy');
console.log(hsm);

var userPin="1234";
console.log("hsm.init("+userPin+")="+hsm.init(userPin));

var soPin = "123456";
var label = "MyLabel";
var emptySlot = true;
console.log("hsm.init2("+label+","+soPin+","+userPin+","+emptySlot+")="
	+hsm.init2(label,soPin,userPin,emptySlot));

emptySlot = false;
console.log("hsm.init2("+label+","+soPin+","+userPin+","+emptySlot+")="
	+hsm.init2(label,soPin,userPin,emptySlot));

console.log("slotId="+hsm.slotId());