<?php
/**
 * All Stripe API calls can be found in this file.
 * To adapt the module to a new Stripe version, look for API calls in this file only.
 *
 * User: ralph
 * Date: 03.01.17
 * Time: 18:24
 */


/**
 * Returns true, if Stripe library is loaded and the API key is set. False otherwise.
 */
function stripe_checkout_library_loaded () {
  $library = libraries_load('stripe');
  if ($library['loaded']) {
    //
    // set the private API key of specific account
    \Stripe\Stripe::setApiKey(_stripe_api_secret_key());
    \Stripe\Stripe::setApiVersion("2019-05-16");
    return true;
  }

  return false;
}


/**
 * Returns the stripe fee for the given amount.
 *
 * @param $amount float   The charged amount.
 * @return float Stripe fee charged with any transaction.
 */
function stripe_checkout_stripe_fee($amount) {
  return round($amount * 0.029) + 30;
}

/**
 * Returns the user defined application fee percentage for the clicked stripe button.
 */
function stripe_checkout_app_fee_percentage($button_id) {
  // get the related fee button of the clicked checkout button
  $app_fee_percentage = 0;
  $stripe_button_relations = array();
  drupal_alter('stripe_checkout_fee_button_relation', $stripe_button_relations);
  if (!empty($stripe_button_relations)) {
    // get fee button field name
    $len = strpos($button_id, '--');
    $stripe_button_name = substr($button_id, 0, $len);
    $fee_button_name = $stripe_button_relations[$stripe_button_name];
    $app_fee_percentages = &stripe_checkout_session_data()['fee_buttons'];
    $app_fee_percentage = $app_fee_percentages[$fee_button_name];
  }
  return $app_fee_percentage;
}


/**
 * Return the registered user object, if available. Otherwise an Exception is thrown.
 *
 * @return mixed|null
 *  Returns the registered user, if any. Otherwise an exception is thrown.
 *
 * @throws \Exception
 */
function stripe_checkout_registered_user() {
  $registered_user = user_is_anonymous() ? false : user_load($GLOBALS['user']->uid);
  return $registered_user;
}


/**
 * Deletes the Stripe customers with the given email address and all its Stripe
 * subscriptions. All related internal subscriptions of the Stripe customers
 * are also deleted. This method guarantees, that only one Stripe customer with
 * the given email address exists containing exactly one subscription.
 *
 * @param $email         string  The email address of a user
 * @param $excl_cust_id  string  A Stripe customer id, which will be excluded
 *                       from deletion.
 *
 * @return int  Number of deleted customers with subscriptions.
 */
function stripe_checkout_delete_subscriptions_for_email($email, $excl_cust_id = '') {
  if (!$email) return 0;
  try {
    $deleted = 0;
    $customers = \Stripe\Customer::all(["email" => $email])->data;
    foreach ($customers as $customer) {
      if ($excl_cust_id === $customer->id) continue;
      if (!$customer->deleted) {
        // deletes the customer and its subscriptions on the Stripe server
        $customer->delete();
      }
      $deleted += pxl_user_stripe_subscription_delete($customer->id);
    }
    return $deleted;
  }
  catch (Exception $e) {
    watchdog('stripe_checkout', 'No Stripe customer with @cust_email found', array('@cust_email' => $email), WATCHDOG_ERROR);
  }
}


/**
 * Deletes the customer and its subscription on the Stripe server for the given
 * registered user, if any and cleanup local subscription table.
 *
 * If the Stripe subscription could not be deleted due to a Stripe exception,
 * the local subscription is also not deleted. So the subscription can be deleted later.
 *
 * @param $registered_user  object
 *    The registered user object.
 */
function stripe_checkout_delete_subscriptions_for_user($registered_user) {
  // do nothing for anonymous user
  if (!$registered_user) return;

  // delete related customer and its subscriptions
  $deleted = stripe_checkout_delete_subscriptions_for_email($registered_user->mail);
  if ($deleted) {
    // inform other modules about new subscription
    module_invoke_all('stripe_checkout_user_unsubscribed', $registered_user);
  }
}


/**
 * Returns the Stripe subscription plan fitting the given parameters.
 * If the plan does not exist yet, a new subscription plan is created and returned.
 *
 * @param $amount
 * @param $currency
 * @param $recurring_billing
 * @return \Stripe\Plan   Created or retrieved Stripe subscription plan.
 */
function stripe_checkout_subscription_plan($amount, $currency, $recurring_billing) {
  //
  // get subscription plan via plan-id, if available
  try {
    $plan_id = 'bge_' . $recurring_billing . '_' . $amount . '_' . $currency;
    $subscription_plan = \Stripe\Plan::retrieve($plan_id);
    return $subscription_plan;
  }
  catch (Exception $e) {
    // no plan with given id found, do nothing here
  }

  //
  // create a new subscription plan
  switch ($amount) {
    case 300:
      $plan_name = t('Basic Income Silver');
      break;
    case 600:
      $plan_name = t('Basic Income Gold');
      break;
    case 1000:
      $plan_name = t('Basic Income Platinum');
      break;
    default:
      $plan_name = t('Basic Income Personal-@amount', array('@amount' => $amount));
      break;
  }

  $subscription_plan = \Stripe\Plan::create([
    "id" => $plan_id,
    "amount" => $amount,
    "currency" => $currency,
    "interval" => _stripe_checkout_recurring_billing_to_interval($recurring_billing),
    "product" => [
      "name" => $plan_name,
    ],
  ]);

  return $subscription_plan;
}


/**
 * Creates a Stripe checkout session from the given settings.
 *
 * @param $stripe_settings
 *
 * @param $subscription_plan \Stripe\Plan
 *   The Stripe\Plan object
 *
 * @return \Stripe\Checkout\Session
 * @throws \Exception
 */
function stripe_checkout_create_session($stripe_settings, $subscription_plan) {
  $amount = $stripe_settings['amount'];
  $currency = $stripe_settings['currency'];
  $recurring_billing = $stripe_settings['recurring_billing'];

  $session_params = [
    "success_url" => $stripe_settings['success_url'],
    "cancel_url" => $stripe_settings['cancel_url'],
    "payment_method_types" => $stripe_settings['payment_method_types'],
    "billing_address_collection" => $stripe_settings['billing_address'],
    "client_reference_id" => $stripe_settings['client_reference_id'],
  ];
  if (isset($stripe_settings['customer_email']) && !empty($stripe_settings['customer_email'])) {
    $session_params["customer_email"] = $stripe_settings['customer_email'];
  }
  if ($recurring_billing === 'one-time') {
    //
    // create line item, if one time billing is set
    $session_params["line_items"] = [[
      'name' => $stripe_settings['name'],
      'description' => $stripe_settings['description'],
      'images' => is_array($stripe_settings['images']) ? $stripe_settings['images'] : [],
      'amount' => $amount,
      'currency' => $currency,
      'quantity' => 1
    ]];
    $session_params["submit_type"] =  $stripe_settings['submit_type'];

  }
  else if ($subscription_plan) {
    //
    // subscription with recurring plan
    $session_params["subscription_data"] = [
      'items' => [[
        'plan' => $subscription_plan->id
      ]]
    ];
  }
  else {
    throw new Exception('No subscription plan available for recurring payment.');
  }

  return \Stripe\Checkout\Session::create($session_params);
}


/* =============================================================================
 *   Stripe checkout webhooks
 * ========================================================================== */

/**
 * Stripe event charge.succeeded webhook implementation.
 *
 * The implementation guarantees the following:
 * - each event is only processed once (idempotency).
 * - the event is only processed, if a customer is attached to the event and a subscription for
 *    this customer exists with a different event id stored.
 * - if no subscription or a subscription with no event id is found,
 *    the event is sent due to a user
 * @param $data
 * @param \Stripe\Event|NULL $event
 */
function stripe_checkout_stripe_api_webhook_charge_succeeded($data, \Stripe\Event $event = NULL) {
  $time = date('d-m-Y H:i:s');
  watchdog('stripe_checkout', 'Webhook charge.succeeded event received: ' . $time, array(), WATCHDOG_DEBUG);

  //
  // Process charge.succeeded event for recurring payments initialized on Stripe
  // create charge params form charge.succeeded event and inform other modules about charge
  $charge = $data->object;
  $stripe_evt_id = $event ? $event->id : 'evt_' . drupal_hash_base64($time);  // create test event id in TEST mode, always perform event

  if ($charge->object === 'charge' && stripe_checkout_library_loaded()) {
    //
    // check for internal subscription and update it, if any
    $app_fee_percentage = 0;
    $plan_id = null;
    $stripe_cust_id = $charge->customer;
    try {
      $customer = \Stripe\Customer::retrieve($stripe_cust_id);
      $charge->customer = $customer;
    }
    catch (Exception $e) {
      // should never happen, email is asked by checkout dialog
    }

    //
    // update internal subscription, if any
    if ($subscription = pxl_user_stripe_subscription_get_subscription($stripe_cust_id)) {
      //
      // check if incoming event has been processed already (idempotency)
      $evt_id = $subscription['stripe_evt_id'];
      if ($evt_id === $stripe_evt_id) {
        $message = 'Incoming event already processed.';
        watchdog('stripe_checkout', 'Webhook charge.succeeded event: @msg', array('@msg' => $message), WATCHDOG_DEBUG);
        return;
      }

      //
      // update the internal subscription of customer with the new Stripe event id
      pxl_user_stripe_subscription_update_subscription($stripe_cust_id, $stripe_evt_id);

      //
      // calculate app_fee from subscription
      $plan_id = $subscription['stripe_plan_id'];
      $app_fee_percentage = $subscription['app_fee_percentage'];
    }

    //
    // Inform all charge.succeeded hooks about subscription charge
    // get charge parameters
    $amount = $charge->amount;
    $params['stripe_api_mode'] = strtolower(variable_get('stripe_api_mode', 'test'));
    $params['plan_id'] = $plan_id;
    $params['stripe_fee'] = stripe_checkout_stripe_fee($amount)/100;
    $params['app_fee'] = $amount * $app_fee_percentage/100;
    module_invoke_all('stripe_checkout_charge_completed', $charge, $params);

  }
  else {
    watchdog('stripe_checkout', 'Webhook charge.succeeded event: Incomplete or wrong charge object: @data', array(
      '@data' => json_encode($data),
    ), WATCHDOG_ERROR);

  }
}

/* --------------------------------------------------
 *  Stripe event - checkout.session.completed
 * --------------------------------------------------*/
/**
 * Stripe event checkout.session.completed webhook implementation
 *
 * @param                    $data
 * @param \Stripe\Event|NULL $event
 */
function stripe_checkout_stripe_api_webhook_checkout_session_completed($data, Stripe\Event $event = NULL) {
  $time = date('d-m-Y H:i:s');
  watchdog('stripe_checkout', 'Webhook checkout.session.completed event received: ' . $time, array(), WATCHDOG_DEBUG);

  //
  // Process checkout.session.completed event initialized on Strip.
  $session = $data->object;
  if ($session->object === 'checkout.session' && stripe_checkout_library_loaded()) {
    //
    // expand customer, payment and client_reference_id in session
    $subscr_intl_id = false;
    try {
      $pay_id = $session->payment_intent;
      $billing_details = null;
      if ($pay_id) {
        $payment = \Stripe\PaymentIntent::retrieve($pay_id);
        $charge = $payment->charges->data[0];
        $amount = $charge->amount;
        $billing_details = $charge ? $charge->billing_details : null;
        $payment->metadata = [
          'stripe_api_mode' => strtolower(variable_get('stripe_api_mode', 'test')),
          'plan_id' => null,
          'stripe_fee' => stripe_checkout_stripe_fee($amount)/100,
          'app_fee' => 0
        ];
        $session->payment_intent = $payment;
      }
      $stripe_cust_id = $session->customer;
      if ($stripe_cust_id) {
        $customer = \Stripe\Customer::retrieve($stripe_cust_id);
        $customer_mail = $customer->email;
        $customer->metadata = $billing_details;
        $session->customer = $customer;
      }

      // unpack client reference id
      if ($session->client_reference_id) {
        $combined_reference_id =  json_decode($session->client_reference_id);
        $session->client_reference_id = isset($combined_reference_id->client_reference_id) ?
          $combined_reference_id->client_reference_id : null;
        $subscr_intl_id = $combined_reference_id->subscr_intl_id ? $combined_reference_id->subscr_intl_id : false;
      }
    }
    catch (Exception $e) {
      // do nothing
      $customer_mail = '';
    }

    //
    // New subscription for a customer: delete all old subscriptions for this customer
    // and update internal subscription with customer id
    if ($subscr_intl_id && $subscription = pxl_user_stripe_subscription_get_subscription($stripe_cust_id, $subscr_intl_id)) {
      //
      // single payment: update application fee
      if ($pay_id) {
        $amount = $payment->amount;
        $session->payment_intent->metadata['app_fee'] = $amount * $subscription['app_fee_percentage']/100;
      }
      else {
        //
        // Subscriptions:
        // update the internal subscription with the customer id and the new Stripe event id
        $stripe_evt_id = $event ? $event->id : 'evt_' . drupal_hash_base64($time);  // create test event id in TEST mode, always perform event
        pxl_user_stripe_subscription_update_subscription($stripe_cust_id, $stripe_evt_id, $subscr_intl_id);

        //
        // delete all Stripe customers with the same email as the anonymous/registered user
        // and its subscriptions
        stripe_checkout_delete_subscriptions_for_email($customer_mail, $stripe_cust_id);

        //
        // successful new Stripe subscription created for registered or anonymous user,
        $registered_user = $subscription['user_id'] ? user_load($subscription['user_id']) : false;
        if ($registered_user) {
          // Registered user
          // inform other modules about new subscription
          $params = _stripe_checkout_get_params_from_subscription($subscription);
          module_invoke_all('stripe_checkout_user_subscribed', $registered_user, $params);
        }
      }

    }

    // inform all subscribed hooks about the completed checkout session
    module_invoke_all('stripe_checkout_session_finished', $session);
  }
  else {
    watchdog('stripe_checkout', 'Webhook checkout.session.completed event: Missing session object: @data', array(
      '@data' => json_encode($data),
    ), WATCHDOG_ERROR);

  }

}


/* --------------------------------------------------
 *  User management and profile form
 * --------------------------------------------------*/
/**
 * User hook called before user is deleted.
 * Guarantees, that the subscriptions on Stripe server are deleted when local user is deleted.
 *
 * @param $account
 */
function stripe_checkout_user_delete($account) {
  // TODO: what if Stripe subscription could not be deleted
  if (stripe_checkout_library_loaded ()) {
    stripe_checkout_delete_subscriptions_for_user($account);
  }
}


/**
 * Alter the user profile form.
 *
 * Alter the user profile to make the subscription of a user manageable.
 */
function stripe_checkout_form_user_profile_form_alter (&$form, &$form_state, $form_id) {
  //
  // show subscription information in the user profile, if available
  $user = $form['#user'];
  $subscriptions = pxl_user_stripe_subscription_get_subscriptions($user->uid);
  if (!empty($subscriptions) && stripe_checkout_library_loaded()) {
    //
    // create table with subscription info of user
    $header = array(
      'subscription' => array('data' => t('Subscription')),
      'created' => array('data' => t('Created')),
      'ops' => array('data' => t('Operations')),
    );
    $rows = array();
    $empty = t('No recurring payment subscription available.');
    $has_valid_subscription = false;

    try {
      foreach ($subscriptions as $subscription) {
        $stripe_cust_id = $subscription['stripe_cust_id'];
        if (empty($stripe_cust_id)) continue;

        $customer = \Stripe\Customer::retrieve(array(
            "id" => $stripe_cust_id,
            "expand" => array("subscriptions")
          )
        );
        if ($customer->deleted) {
          // cleanup db and continue
          pxl_user_stripe_subscription_delete($stripe_cust_id);
          continue;
        }

        // create a row per subscription (should be only one)
        foreach($customer->subscriptions->data as $index => $stripe_subscription) {
          $has_valid_subscription = true;
          $plan = $stripe_subscription->plan;
          $subscription_text = t('@plan: @amount @curr per @interval', array(
              '@plan' => $plan->name,
              '@amount' => $plan->amount/100,
              '@curr' => strtoupper($plan->currency),
              '@interval' => $plan->interval,
            )
          );

          // create delete link
          $uid = $user->uid;
          $del_subscription_url = 'stripe/checkout/user/' . $uid . '/delete_subscription';
          $del_link = l(t('Delete'), $del_subscription_url, array(
            'attributes' => array('class' => array('btn btn-default stripe-user-subscription-delete'), 'title' => t('Delete user subscription'))
          ));

          // create row
          $rows[] = array(
            'subscription' => $subscription_text,
            'created' => format_date($plan->created, 'short'),
            'ops' => $del_link,
          );
        }
      }

      // no valid stripe subscription exists, don't show subscription table
      if (!$has_valid_subscription) return;
    }
    catch (Exception $e) {
      $empty = t('Stripe subscription could not be retrieved. Try later again.');
    }

    // add subscription table
    $form['subscription'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#empty' => $empty,
    );
  }
}



/* --------------------------------------------------
 *  Helper methods
 * --------------------------------------------------*/
/**
 * Transforms stripe interval string into recurring payment string.
 */
function _stripe_checkout_interval_to_recurring_billing($interval) {
  switch ($interval) {
    case 'day':
      return 'daily';
    case 'week':
      return 'weekly';
    case 'month':
    default:
      return'monthly';
    case 'year':
      return 'yearly';
  }
}

/**
 * Transforms recurring payment string into stripe interval string.
 */
function _stripe_checkout_recurring_billing_to_interval($recurring_billing) {
  switch ($recurring_billing) {
    case 'daily':
      return 'day';
    case 'weekly':
      return 'week';
    case 'monthly':
    default:
      return 'month';
    case 'yearly':
      return 'year';
  }
}

/**
 * Get the charge parameters from the subscription.
 *
 * @param $subscription   array
 *    Associative array of subscription parameter
 * @param $plan   object
 *    The subscription plan.
 * @return mixed Associative array of charge parameters or false, if Stripe plan could not be retrieved.
 * Associative array of charge parameters or false, if Stripe plan could not be retrieved.
 */
function _stripe_checkout_get_params_from_subscription($subscription, $plan = false) {
  //
  // get charge parameters from subscription
  if (!$subscription) return false;

  try {
    if (!$plan) {
      $plan_id = $subscription['stripe_plan_id'];
      $plan = \Stripe\Plan::retrieve($plan_id);
    }
    $amount = $plan->amount;
    $currency = $plan->currency;
    $stripe_fee = stripe_checkout_stripe_fee($amount);
    $app_fee = $amount * $subscription['app_fee_percentage'];
    $recurring_billing = _stripe_checkout_interval_to_recurring_billing($plan->interval);
    $stripe_cust_id = $subscription['stripe_cust_id'];

    return array(
      'stripe_api_mode' => strtolower(variable_get('stripe_api_mode', 'test')),
      'recurring_billing' => $recurring_billing,
      'currency' => $currency,
      'amount' => ($amount / 100),
      'stripe_fee' => ($stripe_fee / 100),
      'app_fee' => ($app_fee / 100),
      'stripe_cust_id' => $stripe_cust_id,
    );
  }
  catch (Exception $e) {
    return false;
  }
}
