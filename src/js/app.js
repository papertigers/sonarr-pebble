var Clay = require('pebble-clay');
var clayConfig = require('./config.json');
var clay = new Clay(clayConfig);

Pebble.addEventListener('ready', function() {
  // PebbleKit JS is ready!
  console.log('PebbleKit JS ready!');

  // Update s_js_ready on watch
  Pebble.sendAppMessage({'JSREADY': 1});
});

Pebble.addEventListener('appmessage', function(message) {
  var dict = message.payload;
  if (dict.RequestData) {
    console.log("RECEIEVED REQUEST DATA!!!");
  }
  console.log('received message');
  console.log(JSON.stringify(dict));
});
