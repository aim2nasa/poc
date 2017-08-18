//client.js
const coap = require('../')

req = coap.request('coap://localhost/Rossi')

req.on('response',function(res){
  res.pipe(process.stdout);
})

req.end()
