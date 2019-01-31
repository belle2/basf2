 /*

 Support functions for using Ractive in Validation website

 */

function default_values(keypath) {
    // Default configuration values. This is what we use when we can't find
    // a configuration value in localStorage and sessionStorage.
    // Null is returned, if no default value was set. In this case, also
    // a warning is issued to the log.
    var defaults = {
        "show_overview": true,
        "show_expert_plots": false
    }
    if ( defaults.hasOwnProperty(keypath) ){
        return defaults[keypath];
    }
    else {
        console.warn(
          "No default value found for keypath " + keypath + ". Returning null."
        );
        return null;
    }
}

function convert_string_values ( val ){
    // string representations of true and false need to be converted back
    if ( val == "false") {
        return false;
    }
    else if ( val == "true") {
        return true;
    }
    else if ( val == "null" ){
        return null;
    }
    else if ( val == "undefined"){
        return null;
    }

    return val;
}

function ractive_value_recover( ractive, keypath ) {
    // Finds value corresponding to $keypath either in the localStorage or the
    // sessionStorage. If a non-null value is found in both of them,
    // preference is given to the value frmo the sessionStorage.
    // If the value is found in neither, a default value from the
    // default_values function is taken.
    // The value is then set in ractive using ractive.set

    key = get_storage_id(keypath);

    val_local = convert_string_values(localStorage.getItem(key));
    val_session = convert_string_values(sessionStorage.getItem(key));
    val = null;

    if (val_local !== null && val_session !== null){
        console.debug(
          "Key " + key + " was found in both local storage (value = " + val_local + ") and session storage (value = " + val_session + "). Session storage is taking precedence."
        );
        val = val_session;
    }
    else if ( val_local !== null ){
        console.debug(
          "Key " + key + " was found in local storage (value = " + val_local + ")."
        );
        val = val_local;
    }
    else if ( val_session !== null ){
        console.debug(
          "Key " + key + " was found in session storage (value = " + val_session + ")."
        );
        val = val_session;
    }
    else {
        // simply did not find the key at all
        val_default = default_values(keypath);
        console.debug(
          "key " + key + " was set to default value " + val_default + "."
        );
        val = val_default;
    }

    ractive.set(keypath, val);
}

//function ractive_value_preserve_local( ractive, keypath ) {
//    val = ractive.get(keypath);
//    key = get_storage_id(keypath);
//    localStorage.setItem(key, val );
//    console.log("Storing key " + key + " with value " + val + " to local storage");
//}

function ractive_value_preserve_session( ractive, keypath ) {
    // Saves value associated with $keypath in ractive to sessionStorage.
    val = ractive.get(keypath);
    key = get_storage_id(keypath);
    sessionStorage.setItem(key, val );
    console.debug("Storing key " + key + " with value " + val + " to session storage");
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
                console.log("Ractive setup with " + templateName + " complete");
            }).fail(
            function() {
                alert("Cannot load ractive template " + templateName
                        + " from webserver.");
            });
}


