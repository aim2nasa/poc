//server.js
const coap = require('../')

server = coap.createServer()

server.on('request',function(req,res){
  console.log('===============Request=====================\n');
  console.log(req);
  res.end('Hello ' + req.url.split('/')[1] + '\n')
  console.log('===============Response====================\n');
  console.log(res);
})

server.listen(function(){
  console.log('server started')
})
