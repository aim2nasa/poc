//server.js
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

//디코딩 초기화
var mType = "AES_ECB";
if(hsm.decryptInit(mType,hTagKey)!=0) {
  console.log("hsm.decryptInit("+mType+","+hTagKey+") failed");
  console.log("error message:"+hsm.message());
  process.exit(-1);
}
console.log("hsm.decryptInit("+mType+","+hTagKey+") ok");

server = coap.createServer()

server.on('request',function(req,res){
  //console.log('===============Request=====================\n');
  //console.log(req);
  console.log('req.payload='+req.payload);

  var decBuf = hsm.decrypt(req.payload,req.payload.length);
  console.log("after decoding="+decBuf.toString()+",length="+decBuf.length);

  var auth = false;
  if(req.payload=='AuthorizedClient') auth = true;

  if(auth==true){
    console.log('authentication ok')
  }else{
    console.log('authentication failed')
    res.end('authentication failed\n')
    return
  }

  res.end('Hello ' + req.url.split('/')[1] + '\n')
  //console.log('===============Response====================\n');
  //console.log(res);
})

server.listen(function(){
  console.log('server started')
})
