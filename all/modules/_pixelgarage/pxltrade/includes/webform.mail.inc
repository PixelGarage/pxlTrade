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
 * Dummy email address.
 */
$dummy_address = null;

/**
 * Make sure that all mails have a receiver email-address (needed to be processed)
 * Remark: email field in delivery form is not required, only phone.
 */
function pxltrade_webform_submission_presave($node, &$submission) {
  // find the email component in the webform
  global $dummy_address;

  // get orig node in translation set
  $tnid = $node->tnid ? $node->tnid : $node->nid;
  $orig_form = node_load($tnid);

  // set a dummy email address if no receiver address is given
  foreach ($orig_form->webform['components'] as $key => $data) {
    if ($data['type'] == 'email') {
      if (empty($submission->data[$key][0])) {
        // email address is empty for this submission, set a dummy address
        $submission->data[$key][0] = $dummy_address = 'test@test.tld';
      }
      break;
    }
  }

}

/**
 * Alter the emails for the supplier and the customer before sending.
 * @param $message
 */
function pxltrade_mail_alter(&$message) {
  // alter only webform mails
  if ($message['module'] != 'webform') return;

  //
  // check if webform type has to alter mail message
  $node = $message['params']['node'];
  if (!empty($node->field_content_type) && $node->field_content_type[LANGUAGE_NONE][0]['value'] == 'offer') {
    // it is an offer webform. There's no offer available so nothing to alter
    return;
  }
  else if (empty($node->field_delivery_type)) {
    // not a delivery form, do nothing here
    return;
  }

  //
  // DELIVERY WEBFORM
  // Alter the mail message accordingly
  //
  $tnid = $node->tnid ? $node->tnid : $node->nid;
  $orig_form = node_load($tnid);
  $submission = $message['params']['submission'];
  $eid = $message['params']['email']['eid'];
  $template = variable_get('webform_mail_template_' . $node->nid . '_' . $eid, 'default');
  $offer_nid = _webform_submission_value('offer_nid', $orig_form, $submission);

  if(!($offer = node_load($offer_nid)) || !$offer->status) {
    //
    // don't send email, offer not available anymore (has just now been taken)
    $message['send'] = false;

    // set error message, if no offer was found
    if (!$offer) {
      $error_msg = t('The offer data could not be found. Please contact an administrator.');
      drupal_set_message($error_msg, 'error');
      watchdog('pxlTrade', $error_msg, WATCHDOG_ERROR);
    }
    return;
  }

  //
  // Resolve tokens and adapt mails accordingly
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
    if ($orig_form->nid == 14) {
      // tokens for home delivery
      $to_search += array(
        '%full_name%' => $submission->data[1][0],
        '%phone%' => $submission->data[6][0],
        '%email%' => isset($submission->data[5]) ? $submission->data[5][0] : '',
        '%add_info%' => isset($submission->data[9]) ? $submission->data[9][0] : '',
        '%street_nr%' => $submission->data[2][0],
        '%plz%' => $submission->data[3][0],
        '%location%' => $submission->data[4][0],
      );
    }
    else if ($orig_form->nid == 15) {
      $to_search += array(
        '%full_name%' => $submission->data[4][0],
        '%phone%' => $submission->data[1][0],
        '%email%' => isset($submission->data[2]) ? $submission->data[2][0] : '',
      );
    }

  }

  // replace tokens in subject and message
  // replace tokens with values in mail and complete mail with attachment and address
  $message['subject'] = str_replace(array_keys($to_search), array_values($to_search), $message['subject']);
  $message['body'][0] = str_replace(array_keys($to_search), array_values($to_search), $message['body'][0]);


  //
  // WHATSAPP DELIVERY:
  // No email address is given (only for customers allowed)
  // --> send a whats-app message instead and disable this email message
  global $dummy_address;
  if ($message['to'] == $dummy_address) {
    // don't send email
    $message['send'] = false;

    //
    // send whats-app message to customer with details of the offer provider


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

