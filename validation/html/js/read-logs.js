$(document).ready(function() { 
    
    // Set the on-click event to close the windows for log list and the log reader
    $('.close').click(function(){
	$(this).parent().parent().hide();
    });  

    // Send an AJAX request to retrieve the list of log files
    $('.readlogs').click(function(){
	$.ajax({ 
	    cache: false,
	    url: "/ajax/listlogs",
	    type: "GET",
	    contentType: "application/json",
	    dataType: "json"
	   
	    // Once the request is complete
	}).done(function(json) {

	    // Enable the div which contains the log list and position it
	    // Furthermore make it draggable, but only by the bar on top
	    $('#loglist').show();
	    $('#loglist').css({top:'30px', left:'30px'});
	    $('#loglist').draggable({handle:'#loglist_handle'});	    
	    
	    // Read the data that were retrieved from the AJAX request into an array
	    var data = []
	    for (var pkg in json){
		data.push(pkg);
	    }
	    
	    // Empty the log list, so we can append the log files in a loop
	    $('#loglist_content').html('');

	    // Loop over all packages for which we retrieved log files
	    for (var pkg in data.sort()){
		
		// Append a head line with the title of the package
		$('#loglist_content').append('<strong>'+data[pkg]+'</strong><br>');
		
		// Loop over all files in the package and append them to the list of files
		for (var log in json[data[pkg]].sort()){
		    $('#loglist_content').append('<span class="loglink" name="/logs/'+data[pkg]+'/'+json[data[pkg]][log]+'">'+json[data[pkg]][log]+'</span><br>');
		}
		$('#loglist_content').append('<br>');
		
	    }

	    // If we select one of the logfiles = click on in
	    $('.loglink').click(function(){

		// Enable the div that will show the contents of the log file and position it 
		// Furthermore make it draggable, but only by the bar on top
		$('#logreader').show();
		$('#logreader').css({top:'30px', left:'530px'});
		$('#logreader').draggable({handle:'#logreader_handle'});	    
		
		// Set the title in the top bar so we know which log file we are looking at
		$('#logreader_handle').html($(this).attr("name")+"<div class='close'><img src='images/close.png'></div></div>");

		// Load the contents of the file into the div
		$('#logreader_content').load($(this).attr("name"));
		
		// Again the code to make the #logreader div closable. JavaScript is stupid sometimes.
		$('.close').click(function(){
		    $(this).parent().parent().hide();
		});  
		
	    });
	    
    	});   
	
    });
});
