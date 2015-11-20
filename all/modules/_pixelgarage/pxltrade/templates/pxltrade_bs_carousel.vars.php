<?php
/**
 * Implementation of template preprocess for the pxlTrade bootstrap carousel.
 *
 * Prerequisites: The bootstrap script carousel.js has to be loaded to use this theme.
 */
function template_preprocess_pxltrade_bs_carousel(&$vars) {
  // add carousel classes
  $vars['classes_array'][] = 'carousel slide';

  // add carousel data attributes
  $vars['attributes_array']['data-interval'] = $vars['interval'] ? (string)$vars['interval'] : 'false';
  $vars['attributes_array']['data-pause'] = $vars['pause'] ? 'hover' : 'false';
  $vars['attributes_array']['data-wrap'] = $vars['wrap'] ? 'true' : 'false';
  $vars['attributes_array']['data-keyboard'] = $vars['keyboard'] ? 'true' : 'false';

  // show navigation only for more than one items
  $count = count($vars['items']);
  $vars['navigation'] = $vars['navigation'] && ($count > 1);
}

