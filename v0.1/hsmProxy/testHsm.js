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

console.log("hsm.findKey("+label+","+label.length+")="+hsm.findKey(label,label.length));
console.log("hsm.getFoundKey()="+hsm.getFoundKey());

var name = "SOFTHSM2_CONF";
var value = "./softhsm2-linux.conf";
var overwrite = 1;
console.log("hsm.setEnv("+name+","+value+","+overwrite+")="+hsm.setEnv(name,value,overwrite));

console.log("encryptInit="+hsm.encryptInit("AES_CBC_PAD",9876));
console.log("encryptInit="+hsm.encryptInit("AES_CBC",6543));
console.log("encryptInit="+hsm.encryptInit("AES_ECB",3210));
console.log("encryptInit="+hsm.encryptInit("AES_WRONG",0000));
