//server.js
const coap = require('../')
const hsm = require('../../hsmProxy/build/Release/hsmproxy');

var rtn = hsm.setEnv("SOFTHSM2_CONF","./softhsm2-linux.conf",1)
console.log('hsm.setEnv='+rtn)

server = coap.createServer()

server.on('request',function(req,res){
  //console.log('===============Request=====================\n');
  //console.log(req);
  console.log('req.payload='+req.payload);

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
