-----------------------------------------------------------------------------
Proximity view style plugin
-----------------------------------------------------------------------------
This view style plugin allows to define any kind of view items (rows) as proximity items with a lot of amazing features.

Item placement:
    The items can be placed randomly (with or without overlap according to a given grid), via CSS inside the proximity container
    or as an Isotope layout, e.g. a masonry or packagery. To use one of these Isotope layouts you have to install
    the isotope library in sites/all/libraries. See below for further instructions.

Item action:
    Defined alter hooks (see API) allow a fully customizable item click action.
    The click action response is displayable in a configurable container (modal dialog, page container).
    The modal dialog is implemented with the bootstrap framework, but of course can be themed differently (e.g. jQuery).
    --> See the views-view-proximity theme template.
    The item click action can be performed with AJAX or as full page load. The AJAX calls create deep links,
    so that all user actions create a history.
    The AJAX mode fully supports ajaxified elements in the action response like a form, a button or a link.
    Proximity and AJAX can be disabled seperately.

Proximity effect:
    The items or parts of it can be animated when the mouse pointer approaches them (proximity)
    Proximity effects can be overridden in code (event handler). Implemented effects are opacity and scaling.
    An item description can be displayed, when the pointer is hovering the item.


-----------------------------------------------------------------------------
Proximity API
-----------------------------------------------------------------------------
(see proximity.api.php)

The API methods allow to fully customize the click action of the proximity items. To do that, two alter hooks
have to be implemented in your theme or module:

1) hook_proximity_load_params_alter
The load parameter array defines a specific url parameter for each proximity item.
This parameter is added at the end of the request url (ajax or full page load) and must be unique and URL conform.
The unique parameter defines, which item content has to be loaded from the server (see next api function).

2) hook_proximity_render_item_alter
Implements the item specific content as a render array or html string (already rendered).
The input parameter $param contains the unique load parameter of the item to be returned.


-----------------------------------------------------------------------------
Installation of Isotope Library
-----------------------------------------------------------------------------
If you want to layout proximity items in an Isotope layout, please install the Isotope Library first. Be aware,
that the Isotope library is licensed by David Desandro (Metafizzy). Please pay the license, if you use it!

The Isotope packages is installed as drupal library in the sites/all/libraries folder.
Follow these steps to create the Isotope library:

1. Install and enable Libraries API module (see https://www.drupal.org/project/libraries).
2. Create an folder called 'isotope' in the libraries directory (sites/all/libraries).
3. Download the latest release of the Isotope package from https://github.com/metafizzy/isotope/releases.
4. There are many files in this package. You only need to copy isotope.pkgd.min.js (and isotope.pkgd.js, if debug is needed)
    to the isotope folder.
5. Create a folder called 'layout-modes' in the 'isotope' folder.
6. Copy all needed layout modes of the Isotope control to this folder. Available layouts are (see http://isotope.metafizzy.co/layout-modes.html):
    - masonry (included in Isotope package in folder js/layout-modes)
    - fir-rows (included in Isotope package in folder js/layout-modes)
    - vertical (included in Isotope package in folder js/layout-modes)
    - packagery (separate download http://packery.metafizzy.co)
    - cellsByRow (separate download http://isotope.metafizzy.co/layout-modes/cellsbyrow.html)
    - masonryHorizontal (separate download http://isotope.metafizzy.co/layout-modes/masonryhorizontal.html)
    - fitColumns (separate download http://isotope.metafizzy.co/layout-modes/fitcolumns.html)
    - cellsByColumn (separate download http://isotope.metafizzy.co/layout-modes/cellsbycolumn.html)
    - horizontal (separate download http://isotope.metafizzy.co/layout-modes/horizontal.html)

The final structure looks like this:

sites/all/libraries
  |_ isotope
        |_ layout-modes
        |       |_ masonry.js
        |       |_ vertical.js
        |       |_ fit-rows.js
        |       |_ ...
        |
        |_ ImagesLoaded.pkgd.min.js
        |_ ImagesLoaded.pkgd.js
        |_ Isotope.pkgd.min.js
        |_ Isotope.pkgd.js


-----------------------------------------------------------------------------
Isotope License
-----------------------------------------------------------------------------
The metafizzy-isotope for commercial use has to be licensed.

See http://isotope.metafizzy.co/license.html


The famous Metafizzy-Isotope js-library can be found at the following locations:
Code:           See https://github.com/metafizzy/isotope
Documentation:  See http://isotope.metafizzy.co

