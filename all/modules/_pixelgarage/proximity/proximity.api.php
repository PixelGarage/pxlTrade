<?php
/**
 * Proximity module hooks.
 *
 * User: ralph
 * Date: 22.04.15
 */

/**
 * Alters the proximity item load parameter array.
 *
 * The load parameter array defines a specific url parameter for each proximity item.
 * This parameter is added at the end of the request url and must be unique and URL conform.
 * The unique parameter defines, which item content has to be loaded from
 * the server (see next api function).
 *
 * @param $container_index      int     Index of proximity container (if more than one container exists in one page).
 * @param $view                 array   The view.
 * @param $ajax_load_params     array   Array of ajax load parameters to be altered, one for each proximity item (row)
 *                                      retrieved by the view. Default is the views row index.
 */
function hook_proximity_load_params_alter($container_index, $view, &$ajax_load_params) {
  // Example: the view retrieves nodes as proximity items.
  // Return the node id as the load parameter for each item.
  foreach ($view->result as $id => $item) {
    $ajax_load_params[$id] = $item->nid;
  }
}

/**
 * Implements the item specific content as render array or html string.
 * The input parameter $param contains the unique load parameter of the requested item.
 *
 * @param $container_index      int     Index of proximity container (if more than one container exists in one page).
 * @param $param                string  The item specific load parameter (see also hook_proximity_ajax_load_params_alter).
 * @param $render_item          mixed   The rendered content to be returned to the client. The $render_item should be
 *                                      replaced either by a string (rendered html content), a render array or an integer (error code).
 */
function hook_proximity_render_item_alter($container_index, $param, &$render_item) {
  // Example: the $param variable contains a node id (see hook_proximity_ajax_load_params_alter),
  // return the render array for the specific node
  if ($node= node_load($param)) {
    $render_item = node_view($node);
  }
}
