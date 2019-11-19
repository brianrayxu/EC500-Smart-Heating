
// Modules
//var SerialPort = require('serialport');
//var Readline = require('@serialport/parser-readline')
var express = require('express');
var app = require('express')();
var http = require('http').Server(app);
var io = require('socket.io')(http);
var request = require('request');
var fs = require('fs');
var Engine = require('tingodb')(),
    assert = require('assert');
var db = new Engine.Db('./testdb', {});
var http1 = require('http');
//var fs = require('fs');


    //var db = new Engine.Db('./testdb', {});
    var collection = db.collection("Testquest5");
   // Test stuff --> no serial port

var server = http1.createServer(function (req, res) {
var temp, temp0, temp1, temp2; 

    if (req.method === "GET") {
        res.writeHead(200, { "Content-Type": "text/html" });
        fs.createReadStream("./public/form.html", "UTF-8").pipe(res);
    } else if (req.method === "POST") {
        console.log("message received");
        var body = "";
        req.on("data", function (chunk) {
            body += chunk;
        });

        req.on("end", function(){
            res.writeHead(200, { "Content-Type": "text/html" });
            res.end(body);
            var temp = body.split('\t');
              temp0 = temp[0];
              temp1 = temp[1];
              temp2 = temp[2];
              
              io.emit('message', msg);
              console.log("FOBID " + temp0 + " HUBID " + temp1 + "Passcode " + temp2) ;
              if(temp2 == '5'){
                  console.log("Password correct");
                  const options = {
                  url: 'http://192.168.1.129:80/ctrl',
                  body: "1"
                  };

                  request.put(options);
             }
             else {
                console.log("Password incorrect");
              const options = {
                  url: 'http://192.168.1.129:80/ctrl',
                  body: "0"
                  };

                  request.put(options);

              }
             var options = { timeZone: "America/New_York"}, // you have to know that New York in EST
              estTime = new Date();
          //console.log("Date and Time in EST is: " + estTime.toLocaleString("en-US", options));
          d1 = estTime.toLocaleString("en-US", options);
        collection.insert([{Timestamp:d1,FobID:temp0,HubID:temp1}], {w:1}, function(err, result) {
          assert.equal(null, err);
        });
         // var msg = "1";
         // io.emit('message', msg);
         var msg = [d1,temp0,temp1];
    // Send to client
    io.emit('message', msg);
          console.log("EMIT");
        });
    }

}).listen(2222);

   


// Points to index.html to serve webpage
app.get('/', function(req, res){
  res.sendFile(__dirname + '/index.html');
});


var clientConnected = 0; // this is just to ensure no new data is recorded during streaming
io.on('connection', function(socket){
  console.log('a user connected');
  clientConnected = 0;

  // Call function to stream database data
  //readDB();
  clientConnected = 1;
  socket.on('disconnect', function(){
    console.log('user disconnected');
  });
});



/*function readDB(arg) {

  cursor = collection.find({}).toArray(function(err, item) {
    assert.equal(null, err);
   // assert.equal('1', item.hello);
   var size = item.length;
   for(int i = size - 10; i < size; i++){
    io.emit()
   }
    console.log(item);
  });

  db.createReadStream()
    .on('data', function (data) {
      console.log(data.key, '=', data.value)
      // Parsed the data into a structure but don't have to ...
      var dataIn = {[data.key]: data.value};
      // Stream data to client
      io.emit('message', dataIn);
    })
    .on('error', function (err) {
      console.log('Oh my!', err)
    })
    .on('close', function () {
      console.log('Stream closed')
    })
    .on('end', function () {
      console.log('Stream ended')
    })
}*/

/* User socket connection
io.on('connection', function(socket){
  console.log('a user connected');
  socket.on('disconnect', function(){
    console.log('user disconnected');
  });

});*/
/*function intervalFunc() {
  if (clientConnected == 1) {
var msg = ["1000", "1", "2"];

    // Send to client
    io.emit('message', msg);
 }
}
// Do every 1500 ms
setInterval(intervalFunc, 150);*/

//Listening on localhost:3000
http.listen(3000, function() {
  console.log('listening on *:3000');
});

/*
var request = require('request');
request('192.168.1.123:80/ctrl', function (error, response, body) {
  console.log('error:', error); // Print the error if one occurred
  console.log('statusCode:', response && response.statusCode); // Print the response status code if a response was received
  console.log('body:', body); // Print the HTML for the Google homepage.
});
*/