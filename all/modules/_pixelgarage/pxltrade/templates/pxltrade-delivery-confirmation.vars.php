<?php
/**
 * Date: 20.11.15
 * Time: 22:28
 */

function template_preprocess_pxltrade_delivery_confirmation(&$vars) {
  //
  // check if submission was successful, and return with error messages when not.
  if ($vars['status'] == 'error') {
    $vars['confirmation_message'] = t('<strong>Sorry for the inconvenience!<strong><br> Due to a system error no requests can be processed at the moment. Please try it later again...');
    return;
  }

  //
  // SUBMISSION SUCCESSFUL:
  // set the form specific confirmation message
  $webform = $vars['node'];
  $master = pxltrade_webform_master_form($webform);

  $is_offer_form = !empty($master->field_content_type) && $master->field_content_type[LANGUAGE_NONE][0]['value'];
  $is_delivery_form = !empty($master->field_delivery_type) && $master->field_delivery_type[LANGUAGE_NONE][0]['tid'];

  //
  // set form specific standard confirmation message, if available
  if ($webform->webform['confirmation']) {
    $vars['confirmation_message'] = $webform->webform['confirmation'];
  }
  if ($is_offer_form) {
    // set standard offer form confirmation message
    $vars['confirmation_message'] =  t('<strong>Thank you very much!</strong><br> Your offer has been successfully published. ');

    // work-around for SPAM problem,
    if ($vars['status'] == 'failed_email') {
      $sid = $vars['sid'];
      $submission = webform_get_submission($master->nid, $sid);
      $token = webform_get_submission_access_token($submission);
      $vars['access_token_url'] = url("node/{$master->nid}/submission/{$sid}", array('query' => array('token' => $token), 'absolute' => TRUE));
      $vars['confirmation_message'] .= t('<strong>Please bookmark the following link</strong>, if you want to be able to manage your offer in the future:<br>');
      $vars['status'] = 'success';
    }
    else {
      $vars['confirmation_message'] .= t('Shortly you get an email with further details.');
    }
  }
  else if ($is_delivery_form) {
    // set standard delivery form confirmation message
    $vars['confirmation_message'] =
      t('<strong>Gratulation!</strong><br> You have successfully requested the offer. Shortly you get an email or WhatsApp SMS with further details.');
  }

  //
  // SUBMISSION PARTLY SUCCESSFUL:
  // set corresponding confirmation message and status
  $session_data = &pxltrade_session_data();
  if (isset($session_data['no_contact'])) {
    // display supplier contact info's, if customer could not be contacted via WhatsApp
    $vars['status'] = 'no-contact';
    $vars['confirmation_message'] = $session_data['no_contact'];
    unset($session_data['no_contact']);
  }
  else if (isset($session_data['just_taken'])) {
    // Offer not available anymore: last available offer has been taken by another customer just now
    $vars['status'] = 'just-taken';
    $vars['confirmation_message'] = t('<strong>We are very sorry!</strong><br> The offer has just been taken by another user.');
    unset($session_data['just_taken']);
  }

}


