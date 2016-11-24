<?php
/**
 * The preprocess function for the stripe button fee block.
 */

function template_preprocess_stripe_button_fee_block(&$vars) {
  //
  // define text for stripe button fee block
  $session_data = &stripe_button_session_data();
  $session_data['selected_fee'] = 0.1; // 10% default fee

  $vars['text_top'] = t('We are financed entirely by voluntary contributions. Please support our work with');
  $vars['text_bottom'] = t('of your donation.');
  $vars['text_answer'] = t('<strong>Wow!</strong> Your contribution allows us to keep the platform deinbge.ch up and running.');
  $vars['text_stripe_fee'] = t('2.9% + 0.30 CHF payment transaction fees will always be deducted from your donation.');

  //
  // add js
  $path = drupal_get_path('module', 'stripe_button');
  drupal_add_js($path . '/js/stripe_button_fee.js');
}
