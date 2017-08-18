//client.js
const coap = require('../')
const hsm = require('../../hsmProxy/build/Release/hsmproxy');

var rtn = hsm.setEnv("SOFTHSM2_CONF","./softhsm2-linux.conf",1)
console.log('hsm.setEnv='+rtn)

var userPin="1234";
if(hsm.init(userPin)!=0) {
  console.log("HSM init failure(userPin:"+userPin+")");
  console.log("error message:"+hsm.message());
  process.exit(-1);
}
console.log("hsm.init("+userPin+") ok");
console.log("SlotID="+hsm.slotId());

//Tag키 읽어 오기
var TAG_KEY_LABEL = "GroupTagKey";
console.log("TagKeyLabel="+TAG_KEY_LABEL+",TagKeyLabel length="+TAG_KEY_LABEL.length);

if(hsm.findKey(TAG_KEY_LABEL,TAG_KEY_LABEL.length)!=0){
  console.log("findKey("+TAG_KEY_LABEL+") failed");
  console.log("error message:"+hsm.message());
  process.exit(-1);
}
var hTagKey = hsm.getFoundKey();
console.log("Tag Key found, Found key="+hTagKey);

var coapConnection = {
  host:'localhost',
  pathname: '/payloadTest',
  method: 'GET',
  confirmable:true
}

var req = coap.request(coapConnection)
req.write('AuthorizedClient')

req.on('response',function(res){
  res.pipe(process.stdout);
  //console.log(res)
})

req.end()
