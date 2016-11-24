<?php
/**
 * Contains the API function of the stripe button module.
 *
 * Created: ralph
 */

/**
 * Hook called after the stripe transaction has been successfully performed.
 *
 * Can be used to update Drupal internal data structures.
 *
 * @param $amount              The charged amount in currency
 * @param $stripe_fee          The stripe fee in currency.
 * @param int|The $app_fee     The application fees in currency
 * @param string|The $currency The currency of the charged amount.
 */
function hook_charge_completed($amount, $stripe_fee, $app_fee = 0, $currency = 'CHF') {
  watchdog('stripe_button', '@amount CHF charged including stripe fee = @stripe_fee CHF and application fee = @app_fee CHF', array('@amount' => $amount, '@stripe_fee' => $stripe_fee, '@app_fee' => $app_fee), WATCHDOG_DEBUG);
}
