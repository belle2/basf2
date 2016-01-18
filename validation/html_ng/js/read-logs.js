$(document).ready(function() {

	$.ajax({
            cache: false,
            url: "/validation/ajax/scriptcount",
            type: "GET",
            contentType: "application/json",
            dataType: "json"

            // Once the request is complete
        }).done(function(json) {
			if(json){
				$('#scriptcount').html(json);
				$('#scriptcount').show();
			}
        });

    // Set the on-click event to close the windows for log list and the log reader
    $('.close').click(function() {
        $(this).parent().parent().hide();
    });

    // Send an AJAX request to retrieve the list of log files
    $('.readlogs').click(function() {
        $.ajax({
            cache: false,
            url: "/validation/ajax/listlogs",
            type: "GET",
            contentType: "application/json",
            dataType: "json"

            // Once the request is complete
        }).done(function(json) {

            // Enable the div which contains the log list and position it
            // Furthermore make it draggable, but only by the bar on top
            $('#loglist').show();
            $('#loglist').css({
                top: '30px',
                left: '30px'
            });
            $('#loglist').draggable({
                handle: '#loglist_handle'
            });

            // Read the data that were retrieved from the AJAX request into an array

            $('#loglist_content').html(json);

        });
    });

});

// If we select one of the logfiles = click on in
$('#loglist_content').on('click', '.loglink', function() {

    // Enable the div that will show the contents of the log file and position it 
    // Furthermore make it draggable, but only by the bar on top
    $('#logreader').show();
    $('#logreader').css({
        top: '30px',
        left: '530px'
    });
    $('#logreader').draggable({
        handle: '#logreader_handle'
    });

    // Set the title in the top bar so we know which log file we are looking at
    $('#logreader_handle').html($(this).attr("name") + "<div class='close'><img src='images/close.png'></div></div>");

    // Load the contents of the file into the div
    $('#logreader_content').load($(this).attr("name"));

    // Again the code to make the #logreader div closable. JavaScript is stupid sometimes.
    $('.close').click(function() {
        $(this).parent().parent().hide();
    });

});