<?php
/**
 * Created by PhpStorm.
 * User: ralph
 * Date: 06.02.15
 * Time: 17:23
 */

/**
 * Preprocess variables for the html template.
 */
function pixelgarage_preprocess_html(&$vars) {
  // make sure jQuery UI and effects is loaded for anonymous users
  drupal_add_library('system', 'ui');
  drupal_add_library('system', 'effects');
}

/**
 * Override or insert variables for the page templates.
 */
function pixelgarage_preprocess_page(&$vars) {
  // hide titles on login forms
  pg_login_preprocess_page($vars);

  // replace logo with .svg
  $vars['logo'] = str_replace('.jpg', '.svg', $vars['logo']);
}

/**
 * Adapts the submit and reset button for the exposed form.
 * @param $vars
 */
function pixelgarage_preprocess_views_exposed_form(&$vars) {
  $button = $vars['button'];
  $reset_button = $vars['reset_button'];
  $len = strpos($button, '>') + 1;
  $vars['button'] = substr($button, 0, $len) . "<span class=\"fa fa-check\"></span></button>\n";

  $len = strpos($reset_button, '>') + 1;
  $vars['reset_button'] = substr($reset_button, 0, $len) . "<span class=\"fa fa-times\"></span></button>\n";

}

/* =============================================================================
 *      Front Masonry
 * ========================================================================== */
/**
 * Adapts the proximity view, so that always two randomly chosen testimonials are visible
 * in the masonry.
 */
function pixelgarage_preprocess_views_view_proximity (&$vars) {
  if ($node = _get_donation_node()) {
    $vars['donate_node'] = $node;
  }
}

function _get_donation_node() {
  static $donate_node = null;
  if (empty($donate_node)) {
    // get the published testimonials
    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', 'node')
      ->entityCondition('bundle', 'spenden')
      ->propertyCondition('status', NODE_PUBLISHED)
      ->range(0, 1);
    $result = $query->execute();
    if ($result && !empty($result['node'])) {
      $donate_node = node_load(key($result['node']));
      return node_view($donate_node, 'teaser');
    }
  }
  return false;
}


