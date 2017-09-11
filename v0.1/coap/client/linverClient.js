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

//인코딩 초기화
var mType = "AES_ECB";
if(hsm.encryptInit(mType,hTagKey)!=0) {
  console.log("hsm.encryptInit("+mType+","+hTagKey+") failed");
  console.log("error message:"+hsm.message());
  process.exit(-1);
}
console.log("hsm.encryptInit("+mType+","+hTagKey+") ok");

//인증 문자열 'AuthorizedClient' 암호화
var blockSize = 0x10;
var NumBlock = 2;
const buf = Buffer.alloc(blockSize*NumBlock);
buf.write("AuthorizedClient");

console.log("before encoding="+buf.toString()+",length="+buf.length);
var encBuf = hsm.encrypt(buf,buf.length);
console.log("after encoding="+encBuf.toString()+",length="+encBuf.length);

var coapConnection = {
  host:'localhost',
  pathname: '/version',
  method: 'GET',
  confirmable:true
}

var req = coap.request(coapConnection)
req.write(encBuf)

req.on('response',function(res){
  console.log('server linux version:');
  res.pipe(process.stdout);
  //console.log(res)
})

req.end()