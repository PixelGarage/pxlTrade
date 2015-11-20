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
      'proximity_position_randomly' => 0,
      'proximity_random_grid_cell_size' => 40,
      'proximity_ajax_enabled' => 1,
      'proximity_ajax_container' => 'modal_fix',
      'proximity_mobile_behavior' => 'touch_two',
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
      '#markup' => t('<strong>Proximity style</strong></br>' .
        'Use the proximity style to layout proximity items randomly (or css-positioned) in a container. Each view row contains a Proximity items that ' .
        'reacts with a defined animation effect (scaling, opacity etc.), when the mouse pointer approches them. Additionally for each item an item description ' .
        'can be displayed, when the mouse hovers the item. A mouse click on an item fetches its content via AJAX or page load (see Proximity API for more information) ' .
        'and in case of AJAX displays it in a modal dialog or a full page container. '
      ),
      '#weight' => 5,
    );

    // Add proximity layout options to views style form in dedicated field sets
    $form['positioning'] = array(
      '#type' => 'fieldset',
      '#title' => t('Proximity item positioning'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#weight' => 10,
    );
    $form['ajax'] = array(
      '#type' => 'fieldset',
      '#title' => t('Ajax settings'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#weight' => 20,
    );
    $form['proximity'] = array(
      '#type' => 'fieldset',
      '#title' => t('Proximity settings'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#weight' => 30,
    );

    // define positioning
    $form['proximity_position_randomly'] = array(
      '#type' => 'checkbox',
      '#title' => t('Position items randomly'),
      '#description' => t('Define, if the proximity items are distributed randomly in the container. Deselecting this property means, that the items have to be positioned with css.'),
      '#default_value' => $default_values['proximity_position_randomly'],
      '#weight' => 1,
      '#fieldset' => 'positioning',
    );
    $form['proximity_random_grid_cell_size'] = array(
      '#type' => 'textfield',
      '#title' => t('Size of grid cell'),
      '#description' => t('Define the square grid used to randomly distribute the proximity items (default = 40px). One item per cell.'),
      '#default_value' => $default_values['proximity_random_grid_cell_size'],
      '#size' => 5,
      '#weight' => 2,
      '#fieldset' => 'positioning',
    );

    // define ajax items
    $form['proximity_ajax_enabled'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enable Ajax'),
      '#description' => t('Enables to load the item content via ajax.'),
      '#default_value' => $default_values['proximity_ajax_enabled'],
      '#weight' => 1,
      '#fieldset' => 'ajax',
    );
    $form['proximity_ajax_container'] = array(
      '#type' => 'select',
      '#title' => t('Select Ajax container'),
      '#options' => array(
        'modal_fix' => t('Modal dialog fixed position'),
        'modal_rel' => t('Modal dialog relative to item'),
        'page_cont' => t('Page container'),
      ),
      '#description' => t('Select one of the given ajax containers to display the item content. Options are: modal dialog relative to item position, fixed positioned modal dialog (CSS) and a flexible page container.'),
      '#default_value' => $default_values['proximity_ajax_container'],
      '#weight' => 2,
      '#fieldset' => 'ajax',
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
      '#fieldset' => 'ajax',
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

