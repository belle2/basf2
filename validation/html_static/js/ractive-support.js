/*
Support functions for using Ractive on the validation website
*/

"use strict";


// ============================================================================
// Global variables
// ============================================================================

/**
 * This global variables maps template name to the corresponding ractive object.
 */
let template2ractive = {};

// ============================================================================
// Saving ractive values to local or session storage
// ============================================================================

/**
 * Returns storage id from keypath
 * @param keypath
 * @return {string}
 */
function getStorageId(keypath) {
    return `validation_config_${keypath}`;
}

/**
 * Default configuration values. This is what we use when we can't find
 * a configuration value in localStorage and sessionStorage.
 * @param keypath
 * @returns  Null is returned, if no default value was set, else the default value
 */
function defaultValues(keypath) {
    let defaults = {
        "show_overview": true,
        "show_expert_plots": false
    };
    if (defaults.hasOwnProperty(keypath)) {
        return defaults[keypath];
    } else {
        console.warn(
            `No default value found for keypath '${keypath}'. Returning null.`
        );
        return null;
    }
}

/**
 * Converts a string representation back to boolean/null/string
 * @param val the string value to be converted
 * @returns
 */
function convertStringValues(val) {
    if (val === "false") {
        return false;
    } else if (val === "true") {
        return true;
    } else if (val === "null") {
        return null;
    } else if (val === "undefined") {
        return null;
    }

    return val;
}

/**
 * Finds value corresponding to $keypath in the sessionStorage.
 * If the key does not exist in storage, the default value from the
 * defaultValues function is taken.
 * The value is then set in ractive using ractive.set
 * @param ractive
 * @param keypath
 * @returns None
 */
function ractiveValueRecoverSession(ractive, keypath) {
    let key = getStorageId(keypath);
    let val = convertStringValues(sessionStorage.getItem(key));
    if (val === null) {
        val = defaultValues(keypath);
        console.debug(
            `Did not find key '${key}' in session storage, so it was set ` +
            `to default value '${val}'.`
        );
    }
    ractive.set(keypath, val);
}

/**
 * Finds value corresponding to $keypath either in the localStorage.
 * If the key does not exist in storage, the default value from the
 * defaultValues function is taken.
 * The value is then set in ractive using ractive.set
 * @param ractive
 * @param keypath
 * @returns None
 */
function ractiveValueRecoverLocal(ractive, keypath) {
    let key = getStorageId(keypath);
    let val = convertStringValues(localStorage.getItem(key));
    if (val === null) {
        val = defaultValues(keypath);
        console.debug(
            `Did not find key '${key}' in local storage, so it was set ` +
            `to default value '${val}'.`
        );
    }
    ractive.set(keypath, val);
}

/**
 * Saves value associated with $keypath in ractive to sessionStorage.
 * @param ractive
 * @param keypath
 */
function ractiveValuePreserveSession(ractive, keypath) {
    let val = ractive.get(keypath);
    let key = getStorageId(keypath);
    sessionStorage.setItem(key, val);
    console.debug(`Storing key '${key}' with value '${val}' to session storage`);
}

/**
 * Saves value associated with $keypath in ractive to localStorage.
 * @param ractive
 * @param keypath
 */
function ractiveValuePreserveLocal(ractive, keypath) {
    let val = ractive.get(keypath);
    let key = getStorageId(keypath);
    localStorage.setItem(key, val);
    console.debug(`Storing key '${key}' with value '${val}' to local storage`);

}

// ============================================================================
// Setting up ractive element from template.
// ============================================================================



/**
 * Sets up ractive element from template
 * @param templateName
 * @param element
 * @param data
 * @param onRactiveCreated
 * @param onRactiveTemplateComplete
 * @param onRactiveTeardown
 * @param onRactiveRender
 * @param onRactiveChange
 */
function setupRactive(templateName,
                      element,
                      data,
                      onRactiveCreated,
                      onRactiveTemplateComplete,
                      onRactiveTeardown,
                      onRactiveRender,
                      onRactiveChange) {

    console.log(`Setting up Ractive with template '${templateName}'.`);
    $.get(`templates/${templateName}.html`).then(
        function (html_template) {
            // IMPORTANT: DO NOT CHANGE THIS 'var' TO 'let'!
            var ractive = new Ractive({
                // The `el` option can be a node, an ID, or a CSS selector.
                el: element,
                // We could pass in a string, but for the sake of convenience
                // we're passing the ID of the <script> tag above.
                template: html_template,
                // Here, we're passing in some initial data
                data: data,
                oncomplete: function () {
                    if (typeof onRactiveTemplateComplete != 'undefined') {
                        onRactiveTemplateComplete(ractive);
                    }
                },
                onteardown: function () {
                    if (typeof onRactiveTeardown != 'undefined') {
                        onRactiveTeardown(ractive);
                    }
                },
                onrender: function () {
                    if (typeof onRactiveRender != 'undefined') {
                        onRactiveRender(ractive);
                    }
                },
                onchange: function () {
                    if (typeof onRactiveChange != 'undefined') {
                        onRactiveChange(ractive);
                    }
                }

            });

            if (onRactiveCreated != null) {
                onRactiveCreated(ractive);
            }

            renderLatex();

            template2ractive[templateName] = ractive;

            console.log("Ractive setup with " + templateName + " complete");
        }).fail(
        function () {
            alert(`Cannot load ractive template '${templateName}' from webserver.`);
        });
}
