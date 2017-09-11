//server.js
const coap = require('../')
const hsm = require('../../hsmProxy/build/Release/hsmproxy');
var exec = require("child_process").exec;

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

server = coap.createServer()

server.on('request',function(req,res){
  console.log('req.payload='+req.payload);

  //디코딩 초기화
  var mType = "AES_ECB";
  if(hsm.decryptInit(mType,hTagKey)!=0) {
    console.log("hsm.decryptInit("+mType+","+hTagKey+") failed");
    console.log("error message:"+hsm.message());
    process.exit(-1);
  }
  console.log("hsm.decryptInit("+mType+","+hTagKey+") ok");

  var decBuf = hsm.decrypt(req.payload,req.payload.length);
  console.log("after decoding="+decBuf.toString()+",length="+decBuf.length);

  var authWords = new Buffer(decBuf.length);
  authWords.write('AuthorizedClient');
  console.log('authWords='+authWords.toString());

  var auth = false;
  if(decBuf.toString()==authWords.toString()) auth = true;

  if(auth==true){
    console.log('authentication ok')
  }else{
    console.log('authentication failed')
    res.end('authentication failed\n')
    return
  }
 
  console.log('url='+req.url)
  if(req.url=='/date'){
    console.log('* date\n')
    var interval = setInterval(function() {
      if(res._writableState.ended==true) {
        console.log('writableState ended\n')
        return; //상태가 종료되었으면 아무일도 하지 않는다
      }
      res.write(new Date().toISOString() + '\n')
    }, 1000)
  }else if(req.url='/version'){
    console.log('* version\n')
    exec("uname -amrs",function(error,stdout,stderr){
      if(error!=null) {
        console.log("getLoad:"+error);
      }
      res.write(stdout);
      res.end();
      console.log(stdout);
    })
  }else{
    console.log('* none\n')
  }
})

server.listen(function(){
  console.log('server started')
})
