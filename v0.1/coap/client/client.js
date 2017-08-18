//client.js
const coap = require('../')

var coapConnection = {
  host:'localhost',
  pathname: '/payloadTest',
  method: 'GET',
  confirmable:true
}

var req = coap.request(coapConnection)
req.write('ABCDEFG')

req.on('response',function(res){
  res.pipe(process.stdout);
  //console.log(res)
})

req.end()
