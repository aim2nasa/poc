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

if(hsm.findKey(TAG_KEY_LABEL,TAG_KEY_LABEL.length)!=0){
  console.log("findKey("+TAG_KEY_LABEL+") failed");
  process.exit(-1);
}
var hTagKey = hsm.getFoundKey(); 
console.log("Tag Key found, Found key="+hTagKey);

var SE_KEY_LABEL = "SeKey";
console.log("SeKeyLabel="+SE_KEY_LABEL+",SeKeyLabel length="+SE_KEY_LABEL.length);

if(hsm.findKey(SE_KEY_LABEL,SE_KEY_LABEL.length)!=0){
  console.log("findKey("+SE_KEY_LABEL+") failed");
  process.exit(-1);
}
var hSeKey = hsm.getFoundKey(); 
console.log("SE Key found, Found key="+hSeKey);

var mType = "AES_ECB";
if(hsm.encryptInit(mType,hTagKey)!=0) {
  console.log("hsm.encryptInit("+mType+","+hTagKey+") failed");
  process.exit(-1);
}
console.log("hsm.encryptInit("+mType+","+hTagKey+") ok");
