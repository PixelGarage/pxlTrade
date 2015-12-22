<?php
/**
 * Adapts mails for both recipients, the supplier and the customer.
 *
 * The delivery-form submission send mails to the supplier
 * and the customer of a trade, which are completed here with
 * token replacements.
 *
 * Date: 21.11.15
 * Time: 17:00
 */

/* --------------------------------------------------
 * Webform email adaptation
 * --------------------------------------------------*/
/**
 * Alter the emails for the supplier and the customer before sending.
 * @param $message
 */
function pxltrade_mail_alter(&$message) {
  // alter only webform mails
  if ($message['module'] != 'webform') return;

  //
  // check if webform type has to alter mail message
  // get the master form
  $node = $message['params']['node'];
  $master = pxltrade_webform_master_form($node);
  $is_delivery_form = !empty($master->field_delivery_type) && $master->field_delivery_type[LANGUAGE_NONE][0]['tid'];

  // Alter only delivery webform mails
  if (!$is_delivery_form) return;

  //
  // DELIVERY WEBFORM
  // Alter the mail message accordingly
  //
  // 1) send any mail only, if offer was still available (not just taken by other customer)
  $submission = $message['params']['submission'];
  $session_data = &pxltrade_session_data();
  $just_taken = pxltrade_webform_submission_value($master, 'offer_state', $submission) == 'just taken';
  if (isset($session_data['just_taken']) || $just_taken) {
    $message['send'] = false;
    return;
  }

  $eid = $message['params']['email']['eid'];
  $template = variable_get('webform_mail_template_' . $node->nid . '_' . $eid, 'default');
  $offer_nid = pxltrade_webform_submission_value($master, 'offer_nid', $submission);

  if(!$offer = node_load($offer_nid)) {
    //
    // don't send email, offer not available in db
    $message['send'] = false;

    // set error message, if no offer was found
    $error_msg = t('The offer data could not be found. Please contact an administrator.');
    drupal_set_message($error_msg, 'error');
    watchdog('pxlTrade', $error_msg, array(), WATCHDOG_ERROR);
    return;
  }

  //
  // Resolve tokens and adapt mails accordingly
  // Todo: make submission field search generic (form independant)
  switch ($master->nid) {
    case 14:
      $customer_phone = $submission->data[6][0];
      $customer_email = (!empty($submission->data[5]) && (PXLTRADE_DUMMY_EMAIL_RECIPIENT != $submission->data[5][0])) ? $submission->data[5][0] : '';
      break;
    case 15:
      $customer_phone = $submission->data[1][0];
      $customer_email = (!empty($submission->data[2]) && (PXLTRADE_DUMMY_EMAIL_RECIPIENT != $submission->data[2][0])) ? $submission->data[2][0] : '';
      break;
    default:
      $customer_phone = '';
      $customer_email = '';
      break;
  }
  $to_search = array(
    '%offer_title%' => $offer->title,
    '%first_name%' => $offer->field_address[LANGUAGE_NONE][0]['first_name'],
    '%name%' => $offer->field_address[LANGUAGE_NONE][0]['last_name'],
  );

  if ($template == 'customer') {
    //
    // fill supplier tokens for the customer email
    $to_search += array(
      '%street_nr%' => $offer->field_address[LANGUAGE_NONE][0]['thoroughfare'],
      '%plz%' => $offer->field_address[LANGUAGE_NONE][0]['postal_code'],
      '%location%' => $offer->field_address[LANGUAGE_NONE][0]['locality'],
      '%phone%' => !empty($offer->field_phone) ? $offer->field_phone[LANGUAGE_NONE][0]['value'] : '',
      '%email%' => $offer->field_email[LANGUAGE_NONE][0]['email'],
    );

  }
  else if ($template == 'supplier') {
    //
    // fill customer tokens for the supplier email
    if ($master->nid == 14) {
      // tokens for home delivery
      $to_search += array(
        '%full_name%' => $submission->data[1][0],
        '%phone%' => $customer_phone,
        '%email%' => $customer_email,
        '%add_info%' => !empty($submission->data[9]) ? $submission->data[9][0] : '',
        '%street_nr%' => $submission->data[2][0],
        '%plz%' => $submission->data[3][0],
        '%location%' => $submission->data[4][0],
      );
    }
    else if ($master->nid == 15) {
      // tokens for pick-up delivery
      $to_search += array(
        '%full_name%' => $submission->data[4][0],
        '%phone%' => $customer_phone,
        '%email%' => $customer_email,
      );
    }

  }

  // replace tokens in subject and message
  // replace tokens with values in mail and complete mail with attachment and address
  $message['subject'] = str_replace(array_keys($to_search), array_values($to_search), $message['subject']);
  $message['body'][0] = str_replace(array_keys($to_search), array_values($to_search), $message['body'][0]);

  //
  // add a remark to the supplier mail, if customer could not be contacted (see below)
  if (isset($session_data['no_contact'])) {
    $message['body'][0] .= t("\n\nPS: The customer could not be contacted by email or WhatsApp message. Please contact him with the given phone number!");
  }

  //
  // WHATSAPP DELIVERY:
  // No email address is given
  // --> send a WhatsApp message instead and disable this email delivery
  if ($message['to'] == PXLTRADE_DUMMY_EMAIL_RECIPIENT) {
    //
    // don't send email
    $message['send'] = false;

    //
    // send WhatsApp message to customer with details about the supplier
    $success = false;
    if (!empty($customer_phone) && module_exists('whatsup')) {
      $success = whatsup_send_message($customer_phone, $message['body'][0]);
    }

    //
    // No WhatsApp sending possible
    if (!$success) {
      //
      // No success with no error means, WhatsApp itself or the customer phone number is not reachable on WhatsApp network.
      // --> Display supplier details in confirmation message
      $error = drupal_get_messages('error', true); // empty error message queue, we treat it differently

      if (empty($error)) {
        // number not registered on WhatsApp network
        $session_data['no_contact'] = t('<strong>You cannot be contacted:<br> Your Phone number is not registered on WhatsApp!</strong><br><br>');
      } else {
        // WhatsApp error occurred
        $session_data['no_contact'] = t('<strong>You cannot be contacted:<br> WhatsApp message could not be delivered!</strong><br><br>');
      }
      // add email message replaced with html line-breaks
      $session_data['no_contact'] .= str_replace(PHP_EOL, '<br>', $message['body'][0]);
    }
    else {
      // all OK, reset 'no contact' due to resending emails
      unset($session_data['no_contact']);
    }

    return;
  }

}

/* --------------------------------------------------
 * Webform email admin form alter
 * --------------------------------------------------*/

/**
 * Implements hook_form_FORM_ID_alter().
 * Extends the webform_email_edit_form with new email templates.
 */
function pxltrade_form_webform_email_edit_form_alter(&$form, &$form_state) {
  $node = $form['#node'];
  $eid = $form['eid']['#value'];
  unset($form['template']['template_option']['#options']['custom']);
  $form['template']['template_option']['#options']['customer'] = 'Customer email';
  $form['template']['template_option']['#options']['supplier'] = 'Supplier email';
  $form['template']['template_option']['#default_value'] = variable_get('webform_mail_template_' . $node->nid . '_' . $eid, 'default');
  $form['#submit'][] = 'pxltrade_form_webform_email_edit_form_submit';
}
/**
 * Saves the chosen webform email template.
 */
function pxltrade_form_webform_email_edit_form_submit($form, $form_state) {
  $node = $form['#node'];
  $eid = $form['eid']['#value'];
  variable_set('webform_mail_template_' . $node->nid . '_' . $eid, $form_state['values']['template_option']);
}

