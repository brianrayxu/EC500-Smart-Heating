//By Kyle Martin, David Kirk, and Ayush Upneja

// Modules /////////////////////////////////////////////////////////////////////////////////////////////

//Socket and server
var request = require('request');
var express = require('express');
var app = express();
var http = require('http').createServer(app);
var io = require('socket.io')(http);
var dgram = require('dgram');
var server = dgram.createSocket('udp4');
const mongoose = require('mongoose')

// JSON and url parsers
var bodyParser = require('body-parser');
var jsonParser = bodyParser.json();
var urlencodedParser = bodyParser.urlencoded({ extended: false });

// Port and IP of Host
var PORT = 3333; // external is 3333
var HOST = "192.168.1.102"; // Kyle's Laptop is .102, Pi is .122

//Port and IP of Device
var devPORT = 3333;
var devHOST = "192.168.1.128"

//MONGODB
var testname = "kyle";
const uri = "mongodb+srv://rivicha:abcd@cluster0-udbhw.gcp.mongodb.net/test?retryWrites=true&w=majority";

//user pref
var temp_pref = 24;

var connector = mongoose.connect(uri, {dbName: 'smartheating'});
//MongoDB
var dataSchema = new mongoose.Schema({
  name: String,
  heat: String
  });

  var data = mongoose.model('People', dataSchema, 'people');


//API ENDPOINTS/////////////////////////////////////////////////////////////////////////////////////////

// Publish HTML file
app.get('/', function(req, res){
  res.sendFile(__dirname + '/index.html');
});

// Update water state
app.post('/water', jsonParser, function(req, res){
  //WeatherAPI for outside temp and raining status
  request('http://api.openweathermap.org/data/2.5/weather?q=Boston&APPID=e435e903db94bf859510c2df7c312eb0', { json: true }, (err, res, body) => {
    if (err) { return console.log(err); }
    console.log((body.main.temp - 273.15 | 0) + " Degrees Celsius");
    let temper = (body.main.temp - 273.15 | 0);

    if(body.weather[0].main == "Rain" || (temper > temp_pref + 2 || temper < temp_pref - 2)){ // close window
      server.send("0",devPORT,devHOST,function(error){
        if(error){ 
          console.log('Failed to update state');
        }else{
          console.log('Found device');
        }
      });
    } else { // open window
      server.send("1",devPORT,devHOST,function(error){
        if(error){ 
          console.log('Failed to update state');
        }else{
          console.log('Found device');
        }
      });
    }
  })
    res.send("done");
});


//SOCKET MESSAGE READER/////////////////////////////////////////////////////////////////////////////////

// Send sensor readings to frontend and write JSON to local file
server.on('message', function (message, remote) {
    devPORT = remote.port;
    devHOST = remote.address;

    let tempname = JSON.parse(message.toString())
    console.log(tempname);
    testname = tempname.fob_id;
    console.log(testname);
    if(testname ==  null){
      io.emit('message',  JSON.parse(message.toString()));
    }
    if(testname != null){
      data.find({name: testname}, function(error, comments){
        console.log(comments);
        let tempheatpref = comments[0].heat;
        console.log(tempheatpref);
        tempheatpref = tempheatpref.toString();
        server.send(tempheatpref,devPORT,devHOST,function(error){
          if(error){
            console.log('Failed to update state');
          } else {
            console.log('Found device');
          }
        });
    })

    }
    });

//HOST, SOCKET, AND EXPRESS INITIALIZATIONS/////////////////////////////////////////////////////////////

// User socket connection
io.on('connection', function(socket){
  console.log('a user connected');
  socket.on('disconnect', function(){
    console.log('user disconnected');
  });
});

// Listening on port 3000
http.listen(3000, function() {
  console.log('listening on *:3000');
});

// Create server
server.on('listening', function () {
    var address = server.address();
    console.log('UDP Server listening on ' + address.address + ":" + address.port);
});

// Bind server to port and IP
server.bind(PORT, HOST);
