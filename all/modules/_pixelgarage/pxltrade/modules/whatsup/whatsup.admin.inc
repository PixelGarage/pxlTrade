<?php

/**
 * @file
 * Admin form to set all WhatsApp settings.
 */

define('WHATSAPP_CHAT_API_PATH', '/whatsapp/chat-api');

/**
 * Form to get all required data to communicate via WhatsApp network.
 */
function whatsup_settings_form($form, &$form_state) {
  //
  // Data for the WhatsApp registration process
  $phone_number = variable_get('whatsup_sitewide_phone', '');
  $path = composer_manager_vendor_dir() . WHATSAPP_CHAT_API_PATH;
  $identityExists = file_exists($path . "/src/wadata/id.$phone_number.dat");

  $form['registration_info'] = array(
    '#type' => 'fieldset',
    '#title' => t('Registration Info'),
    '#description' => t('Information about the account to send messages from when using the sitewide WhatsApp account.'),
    '#collapsible' => TRUE,
    '#collapsed' => !empty($phone_number),
  );
  $form['registration_info']['whatsup_sitewide_phone'] = array(
    '#type' => 'textfield',
    '#title' => t('Telephone number'),
    '#description' => t("Telephone number including the country code without '+' or '00'. For instance: 41881112233 (equals +41 088 111 22 33)."),
    '#element_validate' => array('whatsup_phone_validate'),
    '#default_value' => $phone_number,
    '#required' => TRUE,
  );
  $form['registration_info']['whatsup_sitewide_username'] = array(
    '#type' => 'textfield',
    '#title' => t('Username'),
    '#description' => t('This is the username displayed by WhatsApp clients.'),
    '#default_value' => variable_get('whatsup_sitewide_username', variable_get('site_name')),
  );
  $form['registration_info']['whatsup_sitewide_status'] = array(
    '#type' => 'textfield',
    '#title' => t('Status'),
    '#description' => t('Real time indication of what you are doing, so that you can keep up to date with your contacts.'),
    '#default_value' => variable_get('whatsup_sitewide_status', 'Hello, WhatsUp speaking!'),
  );

  //
  // If we have all the info we need, check credentials to display info
  // REMARK: changes password each call
  if ($identityExists && !empty($phone_number)) {
    //
    // Create a instance of Registration manager and check credentials
    try {
      $regManager = new Registration($phone_number);
      $result = $regManager->checkCredentials();

      // save new password
      variable_set('whatsup_sitewide_pwd', $result->pw);

      // create WhatsUp info table
      if (!empty($result)) {
        $rows = array();
        foreach ($result as $key => $value) {
          if ($key == 'expiration' || $key == 'price_expiration') {
            $value = format_date($value, 'long');
          }
          $rows[] = array(
            'data' => array(
              array('data' => $key, 'header' => TRUE),
              array('data' => $value),
            ),
          );
        }
        $form['account_information'] = array(
          '#theme' => 'table',
          '#rows' => $rows,
          '#caption' => t('Account information fetched from WhatsApp'),
        );
      }

    } catch (Exception $e) {
      drupal_set_message($e->getMessage(), 'error');
      watchdog('whatsup', $e->getMessage(), WATCHDOG_ERROR);
    }
  }

  return system_settings_form($form);
}

/**
 * Element validate function to guarantee a correct phone number.
 */
function whatsup_phone_validate($element, &$form_state, $form) {
  if (!empty($element['#value'])) {
    $phone_number = $element['#value'];
    if (!preg_match('/^\d+$/', $phone_number)) {
      form_error($element, t('Wrong number. Do NOT use + or "00" before your number.'));
    }
    if (strlen($phone_number) > 15) {
      form_error($element, t('This phone number has too many digits.'));
    }
    else if (strlen($phone_number) < 6) {
      form_error($element, t('This phone number has too few digits.'));
    }
    else {
      $form_state['values']['whatsup_sitewide_phone'] = $element['#value'];
    }
  }
}

/**
 * Form to register an account (phone number) on the WhatsApp network.
 */
function whatsup_account_registration_form($form, &$form_state) {
  $form['account_reg'] = array(
    '#type' => 'fieldset',
    '#title' => t('WhatsApp account registration'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $form['account_reg']['options'] = array(
    '#type' => 'vertical_tabs',
  );
  $form['account_reg']['options']['request'] = array(
    '#type' => 'fieldset',
    '#title' => t('Request a registration code'),
  );
  $form['account_reg']['options']['request']['method'] = array(
    '#type' => 'select',
    '#title' => t('Choose a method to request a registration code from WhatsApp'),
    '#options' => array(
      '' => t('- Select -'),
      'sms' => t('Send me a SMS'),
      'voice' => t('Call me'),
    ),
  );
  $form['account_reg']['options']['request']['submit'] = array(
    '#type' => 'submit',
    '#submit' => array('whatsup_account_registration_form_request_submit'),
    '#value' => t('Request code'),
  );
  $form['account_reg']['options']['register'] = array(
    '#type' => 'fieldset',
    '#title' => t('Register an account using the code'),
  );
  $form['account_reg']['options']['register']['code'] = array(
    '#type' => 'textfield',
    '#title' => t('Verification code'),
    '#description' => t("To complete your account registration, please enter the 6-digit activation code (without '-')."),
    '#size' => 10,
    '#maxlength' => 6,
  );
  $form['account_reg']['options']['register']['submit'] = array(
    '#type' => 'submit',
    '#submit' => array('whatsup_account_registration_form_code_submit'),
    '#value' => t('Validate code'),
  );
  $form['#validate'][] = 'whatsup_account_registration_form_validate';

  //
  // get phone number and identification
  // if not valid, disable registration buttons
  $phone_number = variable_get('whatsup_sitewide_phone', '');

  if (empty($phone_number)) {
    $form['account_reg']['options']['request']['submit']['#disabled'] = TRUE;
    $form['account_reg']['options']['register']['submit']['#disabled'] = TRUE;
    drupal_set_message(t('First you have to <a href="@url">configure</a> a phone number to register to WhatsApp.', array('%url' => 'admin/pxltrade/whatsup/settings')), 'error');
  }

  return $form;
}

/**
 * Form validation callback for a WhatsUp account registration form.
 */
function whatsup_account_registration_form_validate($form, &$form_state) {
  if ($form_state['values']['op'] == t('Request code') && $form_state['values']['method'] == '') {
    form_set_error('method', t('You must select a valid method.'));
  }
  else if($form_state['values']['op'] == t('Validate code') && strlen($form_state['values']['code']) < 6) {
    form_set_error('code', t('This activation code has too few digits.'));
  }
}

/**
 * Form submit callback for a request a registration code form.
 */
function whatsup_account_registration_form_request_submit($form, &$form_state) {
  $method = $form_state['values']['method'];
  $phone_number = variable_get('whatsup_sitewide_phone', '');

  try {
    //
    $regManager = new Registration($phone_number);
    $result = $regManager->codeRequest($method);

  } catch (Exception $e) {
    drupal_set_message($e->getMessage(), 'error');
    watchdog('whatsup', $e->getMessage(), WATCHDOG_ERROR);

  }

  if ($result && $result->status == 'sent') {
    watchdog('whatsup', 'Registration code from WhatsApp requested');
    drupal_set_message(t("WhatsApp sent you a %method with a code. To complete your account registration, please enter the 6-digit activation code.", array('%method' => $method == 'sms' ? t('SMS') : t('Call'))));
  }
  else if ($result && $result->status == 'ok') {
    drupal_set_message(t('Your credentials are good. No need for a registration code.'));
  }
  else {
    $msg = t('An error occurred during the WhatsApp registration process: %reason%param',
      array('%reason' => $result->reason, '%param' => isset($result->param) ? ': ' . $result->param : ''));
    watchdog('whatsup', $msg, WATCHDOG_ERROR);
    drupal_set_message($msg, 'error');
    if (isset($result->retry_after)) {
      drupal_set_message(t('Retry again after %seconds seconds.', array('%seconds' => $result->retry_after)), 'error');
    }
  }

  unset($form_state['values']['method']);
}

/**
 * Form submit callback for a registration code form.
 */
function whatsup_account_registration_form_code_submit($form, &$form_state) {
  //
  // register account with sent code
  try {
    $phone_number = variable_get('whatsup_sitewide_phone', '');
    $code = $form_state['values']['code'];
    $regManager = new Registration($phone_number);
    $result = $regManager->codeRegister($code);

  } catch (Exception $e) {
    drupal_set_message($e->getMessage(), 'error');
    watchdog('whatsup', $e->getMessage(), WATCHDOG_ERROR);

  }

  if ($result && $result->status == 'ok') {
    // save new password
    variable_set('whatsup_sitewide_pwd', $result->pw);

    $msg = t("The registration code has been successfully completed.");
    watchdog('whatsup', $msg);
    drupal_set_message($msg);
  }
  else {
    $msg = t('An error occurred during the WhatsApp registration process: %reason%param',
      array('%reason' => $result->reason, '%param' => isset($result->param) ? ': ' . $result->param : ''));
    watchdog('whatsup', $msg, WATCHDOG_ERROR);
    drupal_set_message($msg, 'error');
  }

  unset($form_state['values']['code']);
}


/**
 * Form to test a message delivery via WhatsApp network.
 */
function whatsup_test_message_form($form, &$form_state) {
  $form['destination'] = array(
    '#type' => 'textfield',
    '#title' => t('Destination'),
    '#description' => t('Destination telephone number with country code.'),
    '#element_validate' => array('whatsup_destination_phone_validate'),
    '#required' => TRUE,
  );
  $form['type'] = array(
    '#type' => 'radios',
    '#title' => t('Message type'),
    '#description' => t('Select the message type you want to send.'),
    '#options' => array(
      'text' => t('Text'),
//      'image' => t('Image'),
    ),
    '#default_value' => 'text',
  );
  $form['message'] = array(
    '#type' => 'textarea',
    '#title' => t('Text message'),
    '#description' => t('The text message that will be sent to the destination.'),
    '#states' => array(
      'visible' => array(
        ':input[name=type]' => array('value' => 'text'),
      ),
      'required' => array(
        ':input[name=type]' => array('value' => 'text'),
      ),
    ),
  );
  $form['image'] = array(
    '#type' => 'file',
    '#title' => t('Image file'),
    '#description' => t('The image that will be sent to the destination. Allowed extensions are: jpg jpeg gif png.'),
    '#states' => array(
      'visible' => array(
        ':input[name=type]' => array('value' => 'image'),
      ),  
      'required' => array(
        ':input[name=type]' => array('value' => 'image'),
      ),  
    ),
    '#element_validate' => array('whatsup_message_image_validate'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Send'),
  );
  return $form;
}

/**
 * Element validate function.
 */
function whatsup_destination_phone_validate($element, &$form_state, $form) {
  if (!empty($element['#value'])) {
    $value = preg_replace("/[^\d]/", '', $element['#value']);
    $wc = new WhatsappController();
    $dissectPhone = $wc->get_container('util')->dissect($value);
    if (strlen($value) > 15) {
      form_error($element, t('This phone number has too many digits.'));
    }
    else if (strlen($value) < 6) {
      form_error($element, t('This phone number has too few digits.'));
    }
    else if (!$dissectPhone) {
      form_error($element, t('This phone number is not valid.'));
    }
    else {
      $form_state['values']['destination'] = $value;
    }
  }
}

/**
 * Validation callback for image file.
 */
function whatsup_message_image_validate($element, &$form_state, $form) {
  $file = file_save_upload('image', array(
    'file_validate_extensions' => array('jpg jpeg gif png'),
  ));
  // If the file passed validation:
  if ($file) {
    // Move the file, into the Drupal file system
    $uri = 'temporary://whatsup';
    if (!file_prepare_directory($uri)) {
      drupal_mkdir($uri);
    }
    if ($file = file_move($file, $uri)) {
      // Save the file for use in the submit handler.
      $form_state['storage']['image'] = $file;
    }
    else {
      form_error($element, t('Failed to write the uploaded file the site\'s file folder.'));
    }
  }
}

/**
 * Submit callback to actually send the message.
 */
function whatsup_test_message_form_submit($form, &$form_state) {
  $wc = new WhatsappController();
  $wc
    ->setDebug(TRUE)
    ->setCountryCode('ES')
    ->setLangCode('es')
    ->init();
  $success = FALSE;
  $wc
    ->setDestination($form_state['values']['destination']);
  switch ($form_state['values']['type']) {
    case 'image':
      if (isset($form_state['storage']) && isset($form_state['storage']['image'])) {
        $file = $form_state['storage']['image'];
        // We are done with the file, remove it from storage.
        unset($form_state['storage']['image']);
        $success = $wc->sendImage(image_load($file->uri));;
      }
      break;
    
    default:
      $success = $wc->sendMessage($form_state['values']['message']);
      break;
  }
  if ($success) {
    $wc->WaitforReceipt();
    drupal_set_message(t('Your message has been sent.'));
  }
}
