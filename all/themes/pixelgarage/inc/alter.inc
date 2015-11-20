<?php
/**
 * Created by PhpStorm.
 * User: ralph
 * Date: 06.02.15
 * Time: 17:22
 */

/**
 * Alters the menu entries.
 * @param $items
 */
function pixelgarage_menu_alter(&$items) {
  // hides local task tabs
  pg_login_menu_alter($items);
}

/**
 * Holds the current rendering node id.
 */
$rendering_node_id = null;

/**
 * Alters the proximity item load parameter array.
 *
 * The load parameter array defines a specific url parameter for each proximity item.
 * This parameter is added at the end of the ajax url and must be unique and URL conform.
 * The unique parameter defines, which item content has to be loaded from
 * the server (see next api function).
 *
 * @param $container_index      int     Index of proximity container (if more than one container exists in one page).
 * @param $view_result          array   The result array of the view.
 * @param $ajax_load_params     array   Array of ajax load parameters to be altered, one for each proximity item (row)
 *                                      retrieved by the view. Default is the views row index.
 */
function pixelgarage_proximity_load_params_alter($container_index, $view_result, &$ajax_load_params) {
  //
  // Return the term id as the load parameter for each item.
  foreach ($view_result as $id => $item) {
    $ajax_load_params[$id] = $item->tid;
  }
}

/**
 * Returns the item specific content as render array or html string.
 *
 * The parameter $param contains the unique load parameter of the requested item.
 *
 * @param $container_index      int     Index of proximity container (if more than one container exists in one page).
 * @param $param                string  The item specific load parameter (see also hook_proximity_ajax_load_params_alter).
 * @param $render_item          mixed   The rendered content to be returned to the client. The $render_item should be
 *                                      replaced either by a string (rendered html content), a render array or an integer (error code).
 */
function pixelgarage_proximity_render_item_alter($container_index, $param, &$render_item) {
  //
  // The $param contains a term id.
  // Returns a bootstrap slider containing all offer nodes with the given taxonomy term set.
  //
  // get all offer nodes with term set
  global $rendering_node_id;

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'offer')
    ->fieldCondition('field_category', 'tid', $param)
    ->propertyCondition('status', 1);
  $result = $query->execute();

  if (!isset($result['node'])) return;

  // get nodes and create bootstrap slider
  $nodes = node_load_multiple(array_keys($result['node']));
  $rendered_nodes = array();
  foreach ($nodes as $node) {
    $rendering_node_id = $node->nid;
    $rendered_nodes[] = node_view($node);
  }

  $render_item = theme('pxltrade_bs_carousel',
    array(
      'interval' => 0,
      'pause' => 0,
      'wrap' => 1,
      'keyboard' => 1,
      'indicators' => 0,
      'navigation' => 1,
      'id' => drupal_clean_css_identifier($param),
      'items' => $rendered_nodes,
    )
  );
}

function pixelgarage_form_alter(&$form, &$form_state, $form_id) {
  // check for delivery form
  if (isset($form['#node']) && !empty($form['#node']->field_delivery_type) && $form['#node']->field_delivery_type[LANGUAGE_NONE][0]['tid']) {
    //set the offer nid in the hidden field of a delivery form
    global $rendering_node_id;

    if (isset($form['submitted']['offer_nid'])) {
      $form['submitted']['offer_nid']['#value'] = $rendering_node_id;
    }
    //return;

    // ajaxify the delivery form
    $unique_key = drupal_html_id('webform-ajax-wrapper-' . $rendering_node_id);

    // Now we can add a wrapper to our entire form, that will be used as our ajax wrapper:
    $form['#prefix'] = '<div id="' . $unique_key . '">';
    $form['#suffix'] = '</div>';

    // And finally we will ajaxify our submit button:
    $form['actions']['submit']['#ajax'] = array(
      'wrapper' => $unique_key,
      'callback' => 'pixelgarage_webform_ajax_callback',
      'event' => 'click',
      'progress' => array(
        'message' => '',
        'type' => 'throbber',
      ),
    );
    $form['actions']['submit']['#id'] = drupal_html_id('edit-webform-ajax-submit-' . $rendering_node_id);
  }
}

function pixelgarage_webform_ajax_callback($form, &$form_state) {
  $test = 0;

}
