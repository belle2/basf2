$(document).ready(function() {

    // Load the list of revisions
    $("#revisions").load("revisions.html", function() {

        // If we click the regenerate-button:
        $("#regenerate").on("click", function() {

            // Get an array with the values of all checked checkboxes
            var checkedValues = $('[name=revisions]:checked').map(function() {
                return this.value;
            }).get();

            // Show the layer div that indicates that new plots are being created
            $("#outer").show();

            // Now make an AJAX-Request with these values
            $.ajax({
                cache: false,
                url: "/validation/ajax/makeplots",
                type: "POST",
                contentType: "application/json",
                dataType: "json",
                data: JSON.stringify({
                    "input": checkedValues
                })

                // Once the request is done (and was successful)
            }).done(function(json) {

                // Load the 'content.html' which has been modified by the AJAX request
                if (loadcontent() == true) {

                    // Call MathJax again to re-typeset all equations in the 'content.html'
                    MathJax.Hub.Queue(["Typeset", MathJax.Hub]);

                }

                // If there was a problem with the AJAX request:
            }).error(function(xhr, status, error) {
                $("#content").html(xhr.responseText);

                // Always hide the layer
            }).always(function() {
                $("#outer").hide();
            });

        });

    });
});