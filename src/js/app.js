var Clay = require('pebble-clay');
var clayConfig = require('./config.json');
var clay = new Clay(clayConfig);

var requestTimeline = function(url, secret) {
    Pebble.getTimelineToken(function(token) {
      registerWithToken(token, url, secret);
    }, function(error) {
      console.log('Error getting timeline token: ' + error);
    });
};

var registerWithToken = function(token, url, secret) {
  console.log('calling registerWithToken ' + token);
  var req = new XMLHttpRequest();
  req.setRequestHeader('psecret', secret);
  req.open('GET', url + '/register/' + token, true);
  req.onload = function() {
    console.log('Sonarr server registration response: ' + req.responseText);
    //Tell watch app about success
  };
  req.onerror = function() {
    console.log('Sonarr server registration response: ' + req.responseText);
    //Tell watch app about failure
  };
  req.send();
};

Pebble.addEventListener('ready', function() {
  // PebbleKit JS is ready!
  console.log('PebbleKit JS ready!');

  // Update s_js_ready on watch
  Pebble.sendAppMessage({'JSREADY': 1});
});

Pebble.addEventListener('appmessage', function(message) {
  var dict = message.payload;
  if (dict.REQUESTTIMELINE) {
    requestTimeline(dict.SERVERURL, dict.SERVERSECRET);
  }
  console.log('got request from pebble app: ' + JSON.stringify(dict));
});
