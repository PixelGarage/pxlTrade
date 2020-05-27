<?php
/** --------------------------------------------------
 *  DB methods
 * --------------------------------------------------*/
define('STRIPE_CHECKOUT_INTL_SUBSCR_ID_PREFIX', 'subscr_');

/**
 * Adds a new record to the {pxl_user_stripe_subscription} db.
 *
 * @param $uid                int       User id.
 * @param $plan_id            string    Stripe subscription plan id
 * @param $evt_id             string    The event id.
 * @param $app_fee_percentage float     The chosen fee percentage for the subscription. Default = 0.0.
 * @param $cust_id            string    Stripe customer id if any. Default: empty
 *
 * @return array
 *    Returns the added subscription as associative array.
 * @throws \Exception
 */
function pxl_user_stripe_subscription_add($uid, $plan_id, $evt_id, $app_fee_percentage = 0.0, $cust_id = '') {
  $fields = array(
    'user_id' => $uid,
    'stripe_cust_id' => $cust_id,
    'stripe_plan_id' => $plan_id,
    'app_fee_percentage' => $app_fee_percentage,
    'stripe_evt_id' => $evt_id,
  );
  db_insert('pxl_user_stripe_subscription')
    ->fields($fields)
    ->execute();

  return $fields;
}

/**
 * Gets all stored subscriptions for the given user id.
 *
 * @param $uid      int
 *    User id.
 * @return mixed    array
 *    Associative array of subscriptions with keys = {'stripe_cust_id', 'app_fee_percentage', 'stripe_plan_id', 'event_id'}
 *    for the given user or empty array.
 */
function pxl_user_stripe_subscription_get_subscriptions($uid) {
  // get related stripe customer for the given user id
  $results = db_select('pxl_user_stripe_subscription', 'p')
    ->fields('p', array('stripe_cust_id', 'app_fee_percentage', 'stripe_plan_id', 'stripe_evt_id'))
    ->condition('p.user_id', $uid)
    ->execute()
    ->fetchAll(PDO::FETCH_ASSOC);

  return $results;
}

/**
 * Cleanup the internal subscriptions for the given user (anonymous or registered).
 * All entries with an empty stripe customer are deleted when they belong to a
 * registered user or are older than 24 hours (anonymous).
 *
 * @param $user_id int Internal user ID (uid)
 *
 * @return int  Number of deleted rows
 */
function pxl_user_stripe_subscription_cleanup($user_id) {
  $deleted = 0;
  if ($user_id) {
    //
    // delete all internal subscriptions of a registered user without any related stripe customer.
    $deleted = db_delete('pxl_user_stripe_subscription')
      ->condition(db_and()
        ->condition('user_id', $user_id)
        ->condition('stripe_cust_id', ''))
      ->execute();
  }
  else {
    //
    // delete all internal anonymous user payments and subscription older than 24 hours and
    // without any related stripe customer
    $a_day_ago = time() - 86400;
    $subscriptions = pxl_user_stripe_subscription_get_subscriptions(0);
    foreach ($subscriptions as $subscription) {
      if (empty($subscription['stripe_cust_id'])) {
        $evt_id = $subscription['stripe_evt_id'];
        if (strpos($evt_id, STRIPE_CHECKOUT_INTL_SUBSCR_ID_PREFIX) !== 0) continue;
        $subscr_time = str_replace(STRIPE_CHECKOUT_INTL_SUBSCR_ID_PREFIX, '', $evt_id);
        if (intval($subscr_time) < $a_day_ago) {
          $deleted = db_delete('pxl_user_stripe_subscription')
            ->condition('stripe_evt_id', $evt_id)
            ->execute();
        }
      }
    }
  }
  return $deleted;
}


/**
 * Gets the internal subscription of the given stripe customer.
 *
 * REMARK: The combination of customer_id and internal subscription id is a unique key because
 * customer id is unique, subscription event id is unique and one of them is always available
 * in a subscription
 *
 * @param $cust_id  string  Stripe Customer id.
 * @param $subscr_intl_id  string  Internal subscription id
 * @return mixed  Associative array with subscription parameters, false otherwise.
 */
function pxl_user_stripe_subscription_get_subscription($cust_id, $subscr_intl_id = '') {
  if ($subscr_intl_id) {
    // occurs only for new subscriptions: get the internal subscription with the given internal subscription id
    $result = db_select('pxl_user_stripe_subscription', 'p')
      ->fields('p', array('stripe_cust_id', 'user_id', 'app_fee_percentage', 'stripe_plan_id', 'stripe_evt_id'))
      ->condition('stripe_evt_id', $subscr_intl_id)
      ->execute()
      ->fetchAssoc();
    return $result;
  }
  // get the subscription for the given stripe customer
  $result = db_select('pxl_user_stripe_subscription', 'p')
    ->fields('p', array('stripe_cust_id', 'user_id', 'app_fee_percentage', 'stripe_plan_id', 'stripe_evt_id'))
    ->condition('stripe_cust_id', $cust_id)
    ->execute()
    ->fetchAssoc();
  return $result;
}

/**
 * Updates the internal subscription with the Stripe customer id and evt_id.
 *
 * REMARK: The combination of customer_id and internal subscription id is a unique key because
 * customer id is unique, subscription event id is unique and one of them is always available
 * in a subscription
 *
 * @param $cust_id        string    The Stripe customer id.
 * @param $evt_id         string    The Stripe event id.
 * @param $subscr_intl_id string    Internal subscription id
 */
function pxl_user_stripe_subscription_update_subscription($cust_id, $evt_id, $subscr_intl_id = '') {
  $fields = array(
    'stripe_cust_id' => $cust_id,
    'stripe_evt_id' => $evt_id,
  );
  if ($subscr_intl_id) {
    // in this case cust_id is not yet updated in subscription
    db_update('pxl_user_stripe_subscription')
      ->fields($fields)
      ->condition('stripe_evt_id', $subscr_intl_id)
      ->execute();
  }
  else {
    // cust_id is already updated in subscription
    db_update('pxl_user_stripe_subscription')
      ->fields($fields)
      ->condition('stripe_cust_id', $cust_id)
      ->execute();
  }
}

/**
 * Deletes the internal subscription with the given stripe_cust_id.
 *
 * @param $stripe_cust_id  string    The Stripe customer ID.
 *
 * @return int Number of rows that has been deleted
 */
function pxl_user_stripe_subscription_delete($stripe_cust_id) {
  // delete subscription with given stripe customer (unique)
  $deleted = db_delete('pxl_user_stripe_subscription')
    ->condition('stripe_cust_id', $stripe_cust_id)
    ->execute();
  return $deleted;
}



