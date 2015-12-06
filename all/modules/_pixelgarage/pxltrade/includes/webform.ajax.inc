<?php
/**
 * Contains the ajax callback function of an ajaxified webform submission.
 *
 * Date: 20.11.15
 * Time: 14:36
 */

/**
 * The ajax callback function of the ajaxified webform submission (action of the submit button).
 *
 * @param $form
 * @param $form_state
 */
function webform_ajax_callback($form, &$form_state) {
  if (!empty($form_state['webform_completed']) && empty($form_state['save_draft'])) {
    // If user completed his submission, determine what to do.
    $output = _webform_ajax_callback_completed($form, $form_state);
  }
  else {
    // Else, we're just switching page, or saving draft.
    $output = $form;
  }
  return $output;
}

/**
 * AJAX callback helper for a completed webform.
 *
 * Generates a redirect if needed, or displays the appropriate content.
 */
function _webform_ajax_callback_completed($form, $form_state) {
  $output = '';
  $wrapper_id = $form['#node']->webform['webform_ajax_wrapper_id'];
  $sid = $form_state['values']['details']['sid'];

  if ($form['#node']->webform['webform_ajax']) {
    //
    // Unset confirmation message previously set by Webform module, as we displayed it themed.
    // Get messages without clearing queue.
    $status_messages = drupal_get_messages('status', FALSE);
    if (isset($status_messages['status'])) {
      // search message to be erased.
      $confirmation = check_markup($form['#node']->webform['confirmation'], $form['#node']->webform['confirmation_format'], '', TRUE);
      $index = array_search($confirmation, $status_messages['status']);
      if ($index !== FALSE) {
        // If a message is found, remove it from the list
        unset($status_messages['status'][$index]);
        drupal_get_messages('status');
        foreach ($status_messages['status'] as $message) {
          drupal_set_message($message);
        }
      }
    }

    //
    // AJAX: Display webform confirmation always at the webform place (overrule confirmation settings).
    $error_messages = drupal_get_messages('error', FALSE);
    $status = empty($error_messages) ? 'success' : 'error';
    $output = array(
      '#type' => 'markup',
      '#markup' => theme('pxltrade_delivery_confirmation', array('node' => $form['#node'], 'sid' => $sid, 'status' => $status)),
      '#prefix' => '<div id="' . $wrapper_id . '">',
      '#suffix' => '</div>',
    );
  }
  elseif (isset($form_state['redirect'])) {
    // If a form redirect is set, then use it to send a redirect instruction via AJAX.
    ctools_include('ajax');
    ctools_add_js('ajax-responder');
    $redirect = is_array($form_state['redirect']) ? $form_state['redirect'] : array($form_state['redirect'], array());

    // Send two AJAX commands:
    // The first disables the form's submit button, to avoid extra click while waiting for redirect.
    // The second is a redirect command, giving the browser the URL where to go next.
    $output = array(
      '#type' => 'ajax',
      '#commands' => array(
        ajax_command_invoke('#' . $wrapper_id . ' input.form-submit', 'attr', array('disabled', 'disabled')),
        ctools_ajax_command_redirect($redirect[0], 0, $redirect[1]),
      ),
    );
  }

  return $output;
}
