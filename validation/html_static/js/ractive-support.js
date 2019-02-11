 /*

 Support functions for using Ractive in Validation website

 */

function ractive_value_recover( ractive, keypath ) {
    key = get_storage_id(keypath);
    val = localStorage.getItem(key );
    console.log("Restored key " + key + " with value " + val + " from local storage");

    // string representations of true and false need to be converted back
    if ( val == "false") {
        val = false;
    }
    else if ( val == "true") {
        val = true;
    }

    ractive.set(keypath, val);
}

function ractive_value_preserve( ractive, keypath ) {
    val = ractive.get(keypath);
    key = get_storage_id(keypath);
    localStorage.setItem(key, val );
    console.log("Storing key " + key + " with value " + val + " to local storage");
}


function setupRactive(templateName, 
    element, 
    data, 
    onRactiveCreated, 
    onRactiveTemplateComplete, 
    onRactiveTeardown, 
    onRactiveRender, 
    onRactiveChange) {

    console.log("Setting up Ractive with template " + templateName);
    $.get('templates/' + templateName + '.html').then(
            function(html_template) {
                var ractive = new Ractive({
                    // The `el` option can be a node, an ID, or a CSS selector.
                    el : element,
                    // We could pass in a string, but for the sake of convenience
                    // we're passing the ID of the <script> tag above.
                    template : html_template,
                    // Here, we're passing in some initial data
                    data : data,
                    oncomplete : function () {
                        if (typeof onRactiveTemplateComplete != 'undefined') {
                            onRactiveTemplateComplete(ractive);
                        }
                    },
                    onteardown : function() {
                        if (typeof onRactiveTeardown != 'undefined') {
                            onRactiveTeardown(ractive);
                        }
                    },
                    onrender : function() {
                        if (typeof onRactiveRender != 'undefined') {
                            onRactiveRender(ractive);
                        }
                    },
                    onchange : function() {
                        if (typeof onRactiveChange != 'undefined') {
                            onRactiveChange(ractive);
                        }
                    }

                });

                if (onRactiveCreated != null){
                    onRactiveCreated(ractive);
                    }

                    // Reload MathJax, because we might have new LaTeX code in Ractive
                    // elements. It might be a slight overkill to do this every time,
                    // but it's not costly performance wise anyhow, so let's just do it.
                    // In fact, because MathJax replaces the DOMs with pictures, it will
                    // not regenerate things twice.
                    var t0 = performance.now();
                    MathJax.Hub.Queue(["Typeset",MathJax.Hub]);
                    var t1 = performance.now();
                    console.log("Re-Typeset all using MathJax. This took " + (t1-t0) + "ms.")

                console.log("Ractive setup with " + templateName + " complete");
            }).fail(
            function() {
                alert("Cannot load ractive template " + templateName
                        + " from webserver.");
            });
}


