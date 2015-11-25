<?php
/**
 * Implements the default views of the trading platform.
 *
 * Date: 20.11.15
 * Time: 17:09
 */

/**
 * Implements hook_views_default_views().
 */
function pxltrade_views_default_views() {
  // get all views in code (hold in files ending with .view)
  $views = array();
  $files = file_scan_directory(drupal_get_path('module', 'pxltrade') . '/structures/views', '#\.view$#');
  foreach ($files as $filepath => $file) {
    // import view
    require $filepath;

    // Add view to default views listed in the views overview page.
    if (isset($view)) {
      $views[$view->name] = $view;
    }
  }
  return $views;
}

