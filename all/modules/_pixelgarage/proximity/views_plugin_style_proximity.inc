<?php
/**
 * @file
 * Implements the Proximity views style plugin.
 */

class views_plugin_style_proximity extends views_plugin_style {

  /**
   * Set default options.
   */
  function option_definition() {
    $options = parent::option_definition();

    // define default options
    $default_options = array(
      // layout options
      'proximity_layout_mode' => 'none',
      'proximity_random_grid_cell_size' => 40,
      // ajax options
      'proximity_ajax_enabled' => 1,
      'proximity_action_container' => 'modal_fix',
      'proximity_mobile_behavior' => 'touch_two',
      // proximity options
      'proximity_effect_enabled' => 1,
      'proximity_item_selector' => '',
      'proximity_desc_selector' => '',
      'proximity_start_scale' => 1.0,
      'proximity_end_scale' => 1.3,
      'proximity_start_opacity' => 0.3,
      'proximity_end_opacity' => 0.9,
      'proximity_extent' => 80.0,
      'proximity_trans_duration' => 400,
    );

    // Set default values for proximity
    foreach ($default_options as $option => $default_value) {
      $options[$option]['default'] = $default_value;
      if (is_int($default_value)) {
        $options[$option]['bool'] = TRUE;
      }
    }

    return $options;
  }

  /**
   * Includes the proximity option form into the views style form.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    // add options to form filled with saved values
    $default_values = $this->options;

    // display help text first
    $form['proximity_help'] = array(
      '#markup' => t('<br><br><strong>Proximity items view style</strong><br>' .
        'This view style allows to define any kind of Proximity items (view rows) with a lot of amazing features: <br>' .
        '<ul>' .
        '<li><strong>Item placement: </strong>The items can be placed randomly (with or without overlap), in an isotope grid or via CSS inside the proximity container</li>' .
        '<li><strong>Proximity effect: </strong>The items or parts of it can be animated when the mouse pointer approaches them (proximity).</li>' .
        '<li>Proximity effects can be overridden in code (event handler). Implemented effects are opacity and scaling.</li>' .
        '<li>An item description can be displayed, when the pointer is hovering the item.</li>' .
        '<li><strong>Item action: </strong>Defined alter hooks (see API) allow a fully customizable item click action.</li>' .
        '<li>The click action response is displayable in a configurable container (modal dialog, page container).</li>' .
        '<li>The modal dialog is implemented with the bootstrap framework, but of course can be themed differently (e.g. jQuery). See the views-view-proximity theme template.</li>' .
        '<li>The item click action can be performed with AJAX or as full page load. The AJAX calls create deep links, so that all user actions create a history.</li>' .
        '<li>The AJAX mode fully supports ajaxified elements in the action response like a form, a button or a link.</li>' .
        '<li>Proximity and AJAX can be disabled seperately.</li>' .
        '</ul>'
      ),
      '#weight' => 5,
    );

    // Add proximity layout options to views style form in dedicated field sets
    $form['positioning'] = array(
      '#type' => 'fieldset',
      '#title' => t('Item placement'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#weight' => 10,
    );
    $form['action'] = array(
      '#type' => 'fieldset',
      '#title' => t('Item action'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#weight' => 20,
    );
    $form['proximity'] = array(
      '#type' => 'fieldset',
      '#title' => t('Proximity effect'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#weight' => 30,
    );

    // define positioning
    $layout_options = array(
      'none' => t('None (CSS)'),
      'random' => t('Random'),
    );
    if (proximity_isotope_installed()) {
      $layout_options += array(
        'masonry' => t('Masonry'),
        'packery' => t('Packery'),
        'masonryHorizontal' => t('Masonry horizontal'),
        'fitRows' => t('Fit rows'),
        'fitColumns' => t('Fit columns'),
        'cellsByRow' => t('Cells by row'),
        'cellsByColumn' => t('Cells by column'),
        'horizontal' => t('Horizontal'),
        'vertical' => t('Vertical'),
      );
    }
    $form['proximity_layout_mode'] = array(
      '#type' => 'select',
      '#title' => t('Layout mode'),
      '#description' => t("Defines the layout mode for the items. 'None' means no specific layout, 'Random' means a random distribtion in the container, all other are specific Isotope grid layouts."),
      '#default_value' => $default_values['proximity_layout_mode'],
      '#options' => $layout_options,
      '#required' => TRUE,
      '#weight' => 1,
      '#fieldset' => 'positioning',
    );
    $form['proximity_random_grid_cell_size'] = array(
      '#type' => 'textfield',
      '#title' => t('Size of grid cell'),
      '#description' => t('Define the square grid used to randomly distribute the proximity items (default = 40px). One item per cell, the grid size defines, if the item can overlap or not.'),
      '#default_value' => $default_values['proximity_random_grid_cell_size'],
      '#size' => 5,
      '#weight' => 2,
      '#fieldset' => 'positioning',
    );

    // define item action
    $form['proximity_ajax_enabled'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enable Ajax'),
      '#description' => t('Enables the item action to be performed via ajax.'),
      '#default_value' => $default_values['proximity_ajax_enabled'],
      '#weight' => 1,
      '#fieldset' => 'action',
    );
    $form['proximity_action_container'] = array(
      '#type' => 'select',
      '#title' => t('Select action response container'),
      '#options' => array(
        'modal_fix' => t('Modal dialog fixed position'),
        'modal_rel' => t('Modal dialog relative to item'),
        'page_cont' => t('Page container'),
      ),
      '#description' => t('Select one of the given action response containers to display the item content. Options are: modal dialog relative to item position, fixed positioned modal dialog (CSS) and a flexible page container.'),
      '#default_value' => $default_values['proximity_action_container'],
      '#weight' => 2,
      '#fieldset' => 'action',
    );
    $form['proximity_mobile_behavior'] = array(
      '#type' => 'select',
      '#title' => t('Select mobile behavior'),
      '#options' => array(
        'touch_one' => t('One touch'),
        'touch_two' => t('Two touch'),
      ),
      '#description' => t('Select one of the given mobile behaviors: "One touch" opens dialog on first touch, "Two touch" on the second touch.'),
      '#default_value' => $default_values['proximity_mobile_behavior'],
      '#weight' => 3,
      '#fieldset' => 'action',
    );


    // define proximity items
    $form['proximity_effect_enabled'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enable proximity effect'),
      '#description' => t('Enables the proximity effect for all items.'),
      '#default_value' => $default_values['proximity_effect_enabled'],
      '#weight' => 0,
      '#fieldset' => 'proximity',
    );
    $form['proximity_item_selector'] = array(
      '#type' => 'textfield',
      '#title' => t('Item selector'),
      '#description' => t('Define the selector of the item element to be animated when approached. Leave empty to animate the item as a whole (default).'),
      '#default_value' => $default_values['proximity_item_selector'],
      '#size' => 60,
      '#maxlength' => 60,
      '#weight' => 1,
      '#fieldset' => 'proximity',
    );
    $form['proximity_desc_selector'] = array(
      '#type' => 'textfield',
      '#title' => t('Description selector'),
      '#description' => t('Define the selector of the item description element to be shown when approached (relative to item). Leave empty to suppress a description popup.'),
      '#default_value' => $default_values['proximity_desc_selector'],
      '#size' => 60,
      '#maxlength' => 60,
      '#weight' => 2,
      '#fieldset' => 'proximity',
    );
    $form['proximity_start_scale'] = array (
      '#type' => 'textfield',
      '#title' => t('Initial effect position'),
      '#size' => 5,
      '#description'  => t('Define the initial effect position of an item (default effect = scale).'),
      '#default_value' => $default_values['proximity_start_scale'],
      '#weight' => 3,
      '#fieldset' => 'proximity',
    );
    $form['proximity_end_scale'] = array (
      '#type' => 'textfield',
      '#title' => t('Final effect position'),
      '#size' => 5,
      '#description'  => t('Define the final effect position of an item (default effect = scale).'),
      '#default_value' => $default_values['proximity_end_scale'],
      '#weight' => 4,
      '#fieldset' => 'proximity',
    );
    $form['proximity_start_opacity'] = array (
      '#type' => 'textfield',
      '#title' => t('Initial opacity'),
      '#size' => 5,
      '#description'  => t('Define the initial opacity of an item [0,1].'),
      '#default_value' => $default_values['proximity_start_opacity'],
      '#weight' => 5,
      '#fieldset' => 'proximity',
    );
    $form['proximity_end_opacity'] = array (
      '#type' => 'textfield',
      '#title' => t('Final opacity'),
      '#size' => 5,
      '#description'  => t('Define the final opacity of an item [0,1].'),
      '#default_value' => $default_values['proximity_end_opacity'],
      '#weight' => 6,
      '#fieldset' => 'proximity',
    );
    $form['proximity_extent'] = array (
      '#type' => 'textfield',
      '#title' => t('Extent of proximity effect'),
      '#size' => 5,
      '#description'  => t('Define the spatial extent of the proximity effect for an item (unit = px).'),
      '#default_value' => $default_values['proximity_extent'],
      '#weight' => 7,
      '#fieldset' => 'proximity',
    );
    $form['proximity_trans_duration'] = array(
      '#type' => 'textfield',
      '#title' => t('Transition duration'),
      '#description' => t('Duration of the transition in milliseconds when items change position or appearance.'),
      '#default_value' => $default_values['proximity_trans_duration'],
      '#size' => 10,
      '#maxlength' => 5,
      '#weight' => 8,
      '#fieldset' => 'proximity',
    );
  }

}

