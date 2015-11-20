Proximity items view style
==========================
This view style allows to define any kind of items (view rows) as proximity items with a lot of amazing features:

Item placement:
    The items can be placed randomly (with or without overlap according to a given grid) or via CSS inside the proximity container

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


Proximity API
=============
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
