var percent_done;
var connected;
var settings;

var ipAddress;

Pebble.addEventListener("ready", function(e){
  if(typeof localStorage.getItem("octo_ip") !== "undefined"){
    ipAddress = localStorage.getItem("octo_ip");
    console.log("look below");
    console.log(ipAddress);
    getProgress(); 
  }else {
    console.log("no settings");
  }

});

Pebble.addEventListener("showConfiguration", function(e) {
  // Display the config
  Pebble.openURL("http://quillford.github.io/pebble-configs/OctoFace-config.html");
});

Pebble.addEventListener("webviewclosed", function(e) {
  console.log("Configuration window returned: " + e.response);
  settings = JSON.parse(e.response);
  console.log(JSON.stringify(settings));
});

Pebble.addEventListener("appmessage", function(e) {
  console.log(e);
});

function getProgress(){
  if(typeof localStorage.getItem("settings") !== "undefined"){
    settings = JSON.parse(localStorage.getItem("settings"));
    //var octo_ip = settings[octo_ip];
    //var octo_apikey = settings[octo_apikey];
    var octo_ip = "kossel.local";
    var octo_apikey = "B4F2BA2DC33346A1A013D7895D7B04A9";
    var request_url = "http://" + octo_ip + "/api/printer";
    console.log(octo_ip);
    console.log(octo_apikey);
    
    var response;
    var req = new XMLHttpRequest();
    req.open("GET", request_url, true);
    req.setRequestHeader("X-Api-Key", octo_apikey);
    req.onload = function(e) {
      console.log("loaded");
      if(req.readyState == 4 && req.status == 200){
        response = JSON.parse(req.responseText);
        console.log("done!");
        console.log(JSON.stringify(response));
        connected = true;
      }else {
        console.log("something went wrong");
        connected = false;
      }
    };
    req.send(null);
  }else {
    console.log("settings not defined");
    connected = false;
  }
}