//client.js
const coap = require('../')
const hsm = require('../../hsmProxy/build/Release/hsmproxy');

var rtn = hsm.setEnv("SOFTHSM2_CONF","./softhsm2-linux.conf",1)
console.log('hsm.setEnv='+rtn)

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
