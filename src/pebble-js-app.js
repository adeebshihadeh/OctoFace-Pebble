var settings_defined;

// Settings variables
var ipAddress;
var apiKey;

// Variables corresponding to OctoPrint state
var not_connected_state = 0;
var operational_state = 1;
var settings_not_defined_state = 2;

Pebble.addEventListener("ready", function(e){
  if(typeof localStorage.getItem("ipAddress") !== "undefined" && typeof localStorage.getItem("apiKey") !== "undefined"){
    updateSettings();
    settings_defined = true;
    getState(); 
  }else {
    console.log("settings undefined");
    settings_defined = false;
    // send an app message saying that there are no settings
    Pebble.sendAppMessage({"current_state": settings_not_defined_state});
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
  localStorage.setItem("ipAddress", options.ipAddress);
  localStorage.setItem("apiKey", options.apiKey);
  localStorage.setItem("refresh_frequency", options.refresh_frequency);
  updateSettings();
  getState();
  console.log(JSON.stringify(options));
});

Pebble.addEventListener("appmessage", function(e) {
  console.log("app message below");
  console.log(JSON.stringify(e));
  if(e.payload.update === 0){
    console.log("updating");
    getState();
  }
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
          if(response.state.text == "Operational"){
            Pebble.sendAppMessage({"current_state": operational_state});
          }else if(response.state.text == "Printing"){
            getPrintProgress();
          }
          //Pebble.sendAppMessage({"connected": true});
        }else {
          console.log("error with request. status: " + xhr.status);
          Pebble.sendAppMessage({"current_state": not_connected_state});
        }
      }
    };
    xhr.send();
  }else {
    Pebble.sendAppMessage({"current_state": not_connected_state});
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
          var completion_percent = Math.round(response.progress.completion);
          var print_time_left = formatSeconds(response.progress.printTimeLeft);
          console.log(completion_percent + "% complete");
          Pebble.sendAppMessage({"percent": completion_percent, "print_time_left": print_time_left, "filename": response.job.file.name});
        }else {
          console.log("error with request. status: " + xhr.status);
          Pebble.sendAppMessage({"current_state": not_connected_state});
        }
      }
    };
    xhr.send();
  }else {
    Pebble.sendAppMessage({"current_state": not_connected_state});
    console.log("settings not defined");
  }
}

function updateSettings(){
	// Retrieve the user's settings from localStorage
  console.log("presenting localStorage: " + JSON.stringify(localStorage));
	ipAddress = localStorage.getItem("ipAddress");
	apiKey = localStorage.getItem("apiKey");
}

function formatSeconds(seconds){
  var time = new Date(null);
  time.setSeconds(seconds);
  
  return time.toISOString().substr(11, 8);
}