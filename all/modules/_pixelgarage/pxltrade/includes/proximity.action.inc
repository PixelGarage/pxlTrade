<?php
/**
 * Implements the specific proximity click action for the trading platform.
 *
 * Date: 20.11.15
 * Time: 15:23
 */

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
 * @param $view                 object  The view creating the proximity items.
 * @param $ajax_load_params     array   Array of ajax load parameters to be altered, one for each proximity item (row)
 *                                      retrieved by the view. Default is the views row index.
 */
function pxltrade_proximity_load_params_alter($container_index, $view, &$ajax_load_params) {
  //
  // Return the term id as the load parameter for each item.
  foreach ($view->result as $id => $item) {
    $ajax_load_params[$id] = $item->tid;
  }

  // store the exposed filters set in the view
  $session_data = &pxltrade_session_data();
  $session_data['exposed_filters'] = array();
  foreach ($view->exposed_input as $key => $value) {
    $session_data['exposed_filters'][$key] = $value;
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
function pxltrade_proximity_render_item_alter($container_index, $param, &$render_item) {
  //
  // store rendering node id during rendering
  global $rendering_node_id;

  //
  // The $param contains a node id (redirect of all nodes)
  if (strpos($param, 'node-') === 0) {
    $nid = substr($param, 5);
    if ($node = node_load($nid)) {
      $rendering_node_id = $node->nid;
      $render_item = node_view($node); // calls pxltrade_form_alter() for embedded form
    }
    return;
  }

  //
  // The $param contains a term id.
  // Returns a bootstrap slider containing all offer nodes with the given taxonomy term set filtered
  // by the exposed filters.
  $session_data = &pxltrade_session_data();
  $filters = isset($session_data['exposed_filters']) ? $session_data['exposed_filters'] : null;

  //
  // get all offer nodes with the given category and fulfilling all set filters
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'offer')
    ->fieldCondition('field_category', 'tid', $param)
    ->propertyCondition('status', 1)
    ->propertyOrderBy('created', 'DESC'); // newest first

  // set exposed filters
  if (!empty($filters) && array_key_exists('field_address_locality', $filters) && !empty($filters['field_address_locality'])) {
    // locality filter set
    $query->fieldCondition('field_address', 'locality', $filters['field_address_locality']);
  }
  if (!empty($filters) && array_key_exists('field_sex_tid', $filters)) {
    // gender filter set
    $query->fieldCondition('field_sex', 'tid', is_array($filters['field_sex_tid']) ? $filters['field_sex_tid'][0]: $filters['field_sex_tid']);
  }
  $result = $query->execute();

  if (!isset($result['node'])) return;

  // get nodes and create bootstrap slider
  $nodes = node_load_multiple(array_keys($result['node']));
  $rendered_nodes = array();
  foreach ($nodes as $node) {
    $rendering_node_id = $node->nid;
    $rendered_nodes[] = node_view($node); // calls pxltrade_form_alter() for embedded form
  }

  $render_item = theme('pxltrade_bs_carousel',
    array(
      'interval' => 0,
      'pause' => 0,
      'wrap' => 1,
      'keyboard' => 1,
      'indicators' => 1,
      'navigation' => 1,
      'id' => drupal_clean_css_identifier($param),
      'items' => $rendered_nodes,
    )
  );
}

/**
 * Adds additional js/css files for AJAX mode of proximity items.
 */
function pxltrade_preprocess_views_view_proximity(&$vars) {
  drupal_add_js(drupal_get_path('module', 'webform') . '/js/webform.js');
}

/**
 * Alters a trading or delivery form.
 *
 * 1) Adds the offer nid and email to the hidden fields of a delivery form as back reference to the offer.
 * 2) Ajaxify the offer/delivery form submission (allows to display the confirmation at forms place)
 *
 * CAUTION:
 * Make sure, only webform client forms are altered (offer, delivery forms)!
 */
function pxltrade_form_alter(&$form, &$form_state, $form_id) {
  //
  // Check for client webforms (no other forms are altered here)
  if (strpos($form_id, 'webform_client_form') !== 0) return;

  $master = pxltrade_webform_master_form($form['#node']);
  $is_offer_form = !empty($master->field_content_type) && $master->field_content_type[LANGUAGE_NONE][0]['value'];
  $is_delivery_form = !empty($master->field_delivery_type) && $master->field_delivery_type[LANGUAGE_NONE][0]['tid'];

  // Alter only delivery and offer webforms
  if (!$is_delivery_form && !$is_offer_form) return;

  //
  // set hidden fields in delivery form when rendering_node_id is set (during offer rendering)
  global $rendering_node_id;

  if ($is_delivery_form && $rendering_node_id) {
    //
    // Set the offer details in the hidden fields of a delivery form
    if (isset($form['submitted']['offer_nid'])) {
      $form['submitted']['offer_nid']['#value'] = $rendering_node_id;
    }
    if (isset($form['submitted']['offer_email'])) {
      // needed on submission to send email to supplier
      $offer = node_load($rendering_node_id);
      $form['submitted']['offer_email']['#value'] = $offer->field_email[LANGUAGE_NONE][0]['email'];
    }
  }

  //
  // AJAXify delivery and trading forms once (confirmation message replaces form)
  ctools_form_include($form_state, 'webform.ajax', 'pxltrade');
  $id = $rendering_node_id ? $rendering_node_id : $master->nid;
  $unique_wrapper_id = drupal_html_id('webform-ajax-wrapper-' . $id);

  // Now we can add a wrapper to our entire form, that will be used as our ajax wrapper:
  $form['#prefix'] = '<div id="' . $unique_wrapper_id . '">';
  $form['#suffix'] = '</div>';

  // And finally we will ajaxify our submit button (with unique id)
  $form['actions']['submit']['#ajax'] = array(
    'wrapper' => $unique_wrapper_id,
    'callback' => 'pxltrade_webform_ajax_callback',
    'event' => 'click',
    'progress' => array(
      'message' => '',
      'type' => 'throbber',
    ),
  );
  $form['#attached']['js'] = array(
    drupal_get_path('module', 'pxltrade') . '/js/pxltrade.ajax.js',
  );
  $form['actions']['submit']['#id'] = drupal_html_id('edit-webform-ajax-submit-' . $id);

  // prevent redirection of webform confirmation and set webform ajax mode
  $form['#node']->webform['webform_ajax_wrapper_id'] = $unique_wrapper_id;
  $form['#node']->webform['webform_ajax'] = true;

}


