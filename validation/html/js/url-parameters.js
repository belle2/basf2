$(document).ready(function() {

	function getUrlParameter(sParam) {
		var sPageURL = decodeURI(window.location.search.substring(1));
        var sURLVariables = sPageURL.split('&');
        for (var i = 0; i < sURLVariables.length; i++) {
            var sParameterName = sURLVariables[i].split('=');
            if (sParameterName[0] == sParam) {
                return sParameterName[1];
            }
        }
		return null;
   	}
	
	var PackageOverview = getUrlParameter('PackageOverview');
	
	if (PackageOverview=='false') {
		$('input[type="checkbox"][name="overview"]').prop('checked', false);
		$(".nomatrix").each(function(){$(this).addClass("show");});
	}

});