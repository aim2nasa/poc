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
