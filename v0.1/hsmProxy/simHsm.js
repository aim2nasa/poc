// simHsm.js
const hsm = require('./build/Release/hsmproxy');

var name = "SOFTHSM2_CONF";
var value = "./softhsm2-linux.conf";
var overwrite = 1;
console.log("hsm.setEnv("+name+","+value+","+overwrite+")="+hsm.setEnv(name,value,overwrite));

var userPin="1234";
if(hsm.init(userPin)!=0) {
  console.log("HSM init failurei(userPin:"+userPin+")");
  process.exit(-1);
}
console.log("hsm.init("+userPin+") ok");
console.log("SlotID="+hsm.slotId());

var TAG_KEY_LABEL = "GroupTagKey";
console.log("TagKeyLabel="+TAG_KEY_LABEL+",TagKeyLabel length="+TAG_KEY_LABEL.length);

var hTagKey
if(hsm.findKey(TAG_KEY_LABEL,TAG_KEY_LABEL.length)!=0){
  console.log("findKey("+TAG_KEY_LABEL+") failed");
  process.exit(-1);
}
console.log("Key found, Found key="+hsm.getFoundKey());
