function loadcontent() {
    $("#content").load("content.html", function() {
        $.get("content.html", function(data) {

            // Loop over all packages checkboxes and hide those packages that are not selected for display
            $('[name=packages]:not(:checked)').each(function() {
                $("#" + $(this).attr("value")).css("display", "none");
            });

            /* If the content is reloaded, make sure that the description boxes are displayed or not,
	       in accordance with the checkbox  */
            if ($('.displaydescriptions').is(':checked')) {
                $('.wrap_boxes').each(function() {
                    $(this).css("display", "block");
                });
            } else {
                $('.wrap_boxes').each(function() {
                    $(this).css("display", "none");
                });
            }
            if ($('.package_overview').is(':checked')) {
                $('.nomatrix').each(function() {
                    $(this).removeClass("show");
                });
            } else {
                $('.nomatrix').each(function() {
                    $(this).addClass("show");
                });
            }

        }).done(function() {
            $("input.matrix-toggle").click(function() {
                $("span#" + this.value).toggle();
            });

            $("input.plot-size").on("input", function() {
                var range_val = this.value;
                var range_id = this.id;
                $(".imidzes_" + range_id).css("width", (range_val - 2) + "%");
                $("#slidernumber_" + range_id).text(range_val);
                $("input.slidernumber_" + range_id).val(range_val);
            });

            $(".hide").on("click", function() {
                $(".wrap_" + this.id).toggle();
                $(this).text($(this).text() == "(hide)" ? "(show)" : "(hide)");
            });
            $('.selectall').on("click", function() {
                var checked = this.checked;
                $(this).closest('form').find(':checkbox').each(function() {
                    $(this).prop("checked", checked);
                    if (checked) {
                        $("#" + this.value).css("display", "block");
                    } else {
                        $("#" + this.value).css("display", "none");
                    }
                });
            });
            $('.imidz').on("click", function() {
                if (!$(this).attr("id")) {
                    var plot_show_id = this.name;
                } else {
                    var plot_show_id = this.id;
                }
                if ($("." + plot_show_id).css("display") == "none") {
                    if ($('input[type="checkbox"][name="overview"]').prop('checked')) {
                        $(".nomatrix").each(function() {
                            $(this).removeClass("show");
                        });
                    }
                    $("." + plot_show_id).addClass("show");
                }

            });
            $(".hidebutton").on("click", function() {
                $(this).closest(".nomatrix").removeClass("show");
            });

        });
    });
    return true;
}