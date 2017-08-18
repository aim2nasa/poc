//server.js
const coap = require('../')

server = coap.createServer()

server.on('request',function(req,res){
  //console.log('===============Request=====================\n');
  //console.log(req);
  console.log('req.payload='+req.payload);

  var auth = false;
  if(req.payload='AuthorizedClient') auth = true;

  if(auth==true){
    console.log('authentication ok')
  }else{
    console.log('authentication failed')
    return
  }

  res.end('Hello ' + req.url.split('/')[1] + '\n')
  //console.log('===============Response====================\n');
  //console.log(res);
})

server.listen(function(){
  console.log('server started')
})
