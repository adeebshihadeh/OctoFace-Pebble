var percent_done;
var current_state;
var settings;
var print_time_left;
var print_time_elapsed;

var settings_defined;

// Settings variables
var ipAddress;
var apiKey;

// Variables corresponding to OctoPrint state
var connection_error_state = 0;
var operational_state = 1;
var printing_state = 2;
var settings_not_defined_state = 3;


Pebble.addEventListener("ready", function(e){
  if(typeof localStorage.getItem("ipAddress") !== "undefined" && typeof localStorage.getItem("apiKey") !== "undefined"){
    updateSettings();
    settings_defined = true;
    getState(); 
  }else {
    console.log("settings undefined");
    settings_defined = false;
    // send an app message saying that there are no settings
    // also say that we are not connected
  }
});

Pebble.addEventListener("showConfiguration", function(e) {
  // Display the config
  Pebble.openURL("http://quillford.github.io/pebble-configs/OctoFace-config.html");
});

Pebble.addEventListener("webviewclosed", function(e) {
  console.log("Configuration window returned: " + e.response);
  var options = JSON.parse(decodeURIComponent(e.response));
  localStorage.setItem("ipAddress", options["ipAddress"]);
  localStorage.setItem("apiKey", options["apiKey"]);
  updateSettings();
  console.log(JSON.stringify(options));
});

Pebble.addEventListener("appmessage", function(e) {
  console.log("app message below");
  console.log(e);
});

function getState(){
  if(settings_defined){
    var request_url = "http://" + ipAddress + "/api/printer";
    console.log("sending state request to " + ipAddress + " with apikey " + apiKey);
    
    var response;
    var xhr = new XMLHttpRequest();
    xhr.open("GET", request_url, true);
    xhr.setRequestHeader("X-Api-Key", apiKey);
    xhr.onload = function(e) {
      if(xhr.readyState == 4){
        if(xhr.status == 200){
          response = JSON.parse(xhr.responseText);
          console.log("successful request");
          console.log(JSON.stringify(response));
          if(response["state"]["text"] == "Printing"){
            getPrintProgress();
          }
          Pebble.sendAppMessage({"connected": true}); 
        }else {
          console.log("error with request. status: " + xhr.status);
          Pebble.sendAppMessage({"connected": false});
        }
      }
    };
    xhr.send();
  }else {
    Pebble.sendAppMessage({"connected": false});
    console.log("settings not defined");
  }
}

function getPrintProgress(){
  if(settings_defined){
    var request_url = "http://" + ipAddress + "/api/job";
    console.log("sending job info request to " + ipAddress + " with apikey " + apiKey);
    
    var response;
    var xhr = new XMLHttpRequest();
    xhr.open("GET", request_url, true);
    xhr.setRequestHeader("X-Api-Key", apiKey);
    xhr.onload = function(e) {
      if(xhr.readyState == 4){
        if(xhr.status == 200){
          response = JSON.parse(xhr.responseText);
          console.log("successful request");
          console.log(JSON.stringify(response));
          console.log(response["progress"]["completion"] + "complete");
          Pebble.sendAppMessage({"percent": Math.round(response["progress"]["completion"])});
        }else {
          console.log("error with request. status: " + xhr.status);
          Pebble.sendAppMessage({"connected": false});
        }
      }
    };
    xhr.send();
  }else {
    Pebble.sendAppMessage({"connected": false});
    console.log("settings not defined");
  }
}


function updateSettings(){
	// Retrieve the user's settings from localStorage
	ipAddress = localStorage.getItem("ipAddress");
	apiKey = localStorage.getItem("apiKey");
}