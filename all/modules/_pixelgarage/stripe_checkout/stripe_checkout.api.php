<?php
/**
 * Contains the API function of the stripe checkout module.
 *
 * Created: ralph
 */

/**
 * This hook allows to define all relations between a stripe checkout button field and a fee button field.
 *
 * Use this hook to relate a stripe checkout button with a fee button, so
 * that the correct fee percentage is used for the clicked stripe button.
 *
 * REMARK: Use machine names of button fields, but replace '_' with '-'.
 * Delta values of fields have not to be considered.
 *
 * @param $stripe_button_relations  array
 *    Empty array to be filled with all stripe button - fee button relations.
 */
function hook_stripe_checkout_fee_button_relation_alter(&$stripe_button_relations) {
  $stripe_button_relations = ['stripe-checkout-button-field-name' => 'fee-button-field-name'];
}

/**
 * This hook alters the feedback associative array to provide a specific feedback for each selectable fee percentage.
 * Use it to give a positive feedback to the user and explain, what the selected fee is used for.
 *
 * REMARK: Keys are the defined fee percentages (string) of a fee button.
 *
 * @param $feedbacks  array
 *    An associative array to be altered with a feedback pro selectable fee percentage.
 * @param $fee_button_id   string
 *    The id of the Stripe fee button field.
 */
function hook_stripe_checkout_fee_button_select_feedback_alter(&$feedbacks, $fee_button_id) {
  if ($fee_button_id == 'field_name_xy') {
    $feedbacks += [
      '0.0' => t('<strong>Too bad!</strong> We are entirely financed by voluntary commission. Your contribution would make a difference.'),
      '0.05' => t('<strong>Thank you!</strong> Your contribution shows us that you appreciate our work.'),
      '0.1' => t('<strong>Wow!</strong> Your contribution allows us to keep this platform up and running.'),
      '0.2' => t('<strong>Amazing!</strong> Your contribution enables us to enhance the functionality of this platform.'),
      '0.3' => t('<strong>Absolutely awesome!</strong> We are very grateful that you honor our work so generously.'),
    ];
  }
}

/**
 * Allows to alter the session parameters, before the checkout session is created.
 *
 * Can be used to store contextual data of the internal system to complete a checkout session.
 *
 * REMARK:
 * - amount, currency and recurring billing cannot be changed (will be reset to original values)
 * - use 'client_reference_id' to store a reference to an internal context object.
 *   This context object can be retrieved again in the checkout.session.completed hook.
 * - store images of the product in $session_params['images']. Must be an array.
 *
 * See also hook_stripe_checkout_session_completed.
 *
 * @param $session_params array
 *    The session paramter array enhanced with some additional data,
 *    e.g. stripe fee and application fee percentage.
 */
function hook_stripe_checkout_session_params_alter(&$session_params) {
  // for example: add images of the item to be payed to the session parameters
}

/**
 * This hook is called right before the checkout session is started.
 * All session parameters are made available in the session parameter.
 * To prevent the session creation, throw an exception with a message telling the reason of the interruption.
 *
 * @param $session_params
 *    The associative array with the following session parameters as key-value pairs:
 *      stripe_api_mode:  The stripe API mode, e.g. test | live.
 *      currency:         The currency of the charged amount.
 *      amount:           The charged amount in currency.
 *      stripe_fee:       The calculated stripe fee.
 *      app_fee:          The application fee.
 *      recurring_billing:The recurring payment interval, e.g. daily|weekly|monthly|yearly
 * @throws \Exception
 */
function hook_stripe_checkout_session_starting($session_params) {
  if (empty($session_params['currency'])) {
    throw new Exception('No currency set. No charge can be performed.');
  }
}

/**
 * This hook is called via webhook when the checkout session has been completed
 * on the Stripe server. Use it to complete the checkout payment in the local system.
 *
 * REMARK: A webook call opens a new browser session, so no session setttings are
 * available anymore. To retrieve contextual data of the internal system to complete
 * the checkout session, you can use the 'client_reference_id'.
 * This is a unique string to reference the Checkout Session, e.g. it can be a customer ID,
 * a cart ID, or similar, and can be used to reconcile the session with your internal systems.
 *
 * See also hook_stripe_checkout_session_params_alter.
 *
 * @param $session \Stripe\Checkout\Session
 *    The Stripe session object
 */
function hook_stripe_checkout_session_finished($session) {
  if ($session->object !== 'checkout.session') return;
}


/**
 * This hook is called each time a stripe subscription is charged successfully
 * on the Stripe server (also recurring payments). It's called via webhook and
 * therefore opens a new browser session.
 *
 * In a checkout process this event is called before the checkout.session.completed event.
 *
 * @param $charge       \Stripe\Charge
 *                      The Stripe Charge object with expanded \Stripe\Customer object
 * @param $params       array
 *                      Associative array with additional parameters. [stripe_fee, app_fee, subscription plan id].
 */
function hook_stripe_checkout_charge_completed($charge, $params) {
  $billing = $charge->billing_details;
}

/**
 * This hook is called when the recurring payment subscription (Stripe) of the user is
 * successfully finished.
 * @param $user object
 *    The subscribed user object.
 * @param $params  array
 *    An associative array with the following charge parameters as key-value pairs:
 *      stripe_api_mode:  The stripe API mode, e.g. test | live.
 *      currency:         The currency of the charged amount.
 *      amount:           The charged amount in currency.
 *      stripe_fee:       The stripe fee in currency.
 *      app_fee:          The application fees in currency.
 *      recurring_billing:The recurring payment interval, e.g. daily|weekly|monthly|yearly
 *      stripe_cust_id:   The Stripe customer id holding the subscription.
 *
 */
function hook_stripe_checkout_user_subscribed($user, $params) {
  // update user after subscription, if needed
}

/**
 * This hook is called after the recurring payment subscription (Stripe) of the user has been deleted.
 * @param $user object
 *    The subscribed user object.
 */
function hook_stripe_checkout_user_unsubscribed($user) {
  // update user after subscription, if needed
}

