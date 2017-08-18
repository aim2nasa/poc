//client.js
const coap = require('../')
const hsm = require('../../hsmProxy/build/Release/hsmproxy');

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
