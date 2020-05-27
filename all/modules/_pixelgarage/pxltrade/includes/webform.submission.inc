<?php
/**
 * Contains submission functions and hooks.
 *
 * Date: 13.12.15
 * Time: 14:45
 */

/* --------------------------------------------------
 * Webform submission helpers
 * --------------------------------------------------*/
/**
 * Gets the submitted value for a specific webform component.
 *
 * @param object $master
 *    The webform master node containing the component to be read.
 * @param string $form_key
 *    The defined form key of the component.
 * @param object $submission
 *    The form submission with the submitted values.
 * @return string The value of the webform component or an empty string, if not set.
 *    The value of the webform component or an empty string, if not set.
 */
function pxltrade_webform_submission_value($master, $form_key, $submission) {
  foreach ($master->webform['components'] as $key => $data) {
    if ($data['form_key'] == $form_key) {
      return !empty($submission->data[$key]) ? $submission->data[$key][0] : '';
    }
  }
}


/* --------------------------------------------------
 * Webform submission and related offer adaptations
 * --------------------------------------------------*/

/**
 * Called for each webform submission before it is saved
 *
 * 1) Create or update the submission's related offer node.
 * Offer webform: add a new offer, if not existing yet.
 * Delivery webform: update the related offer (status, remaining offers)
 *
 * 2) Make sure that all email form fields have a an email-address added.
 * Remark: Email field in delivery form is required, not phone.
 */
function pxltrade_webform_submission_presave($node, &$submission) {
  // get the master form
  $master = pxltrade_webform_master_form($node);

  // make sure that each webform has a filled email address (offer and delivery forms)
  // set a dummy email address if no recipient address is given
  foreach ($master->webform['components'] as $key => $data) {
    if ($data['type'] == 'email') {
      if (empty($submission->data[$key][0])) {
        // email address is empty for this submission, set a dummy address
        $submission->data[$key][0] = PXLTRADE_DUMMY_EMAIL_RECIPIENT;
      }
      break;
    }
  }

  //
  // reset session data for mail and offer handling
  $session_data = &pxltrade_session_data();
  unset($session_data['no_contact']);
  unset($session_data['just_taken']);

  //
  // complete submission and create / update related offer nodes
  _pxltrade_manage_submission_related_offer($master, $submission);

}

/**
 * Creates or updates the submission related offer node depending on the submission form type.
 *
 * @param $master     object  The master form node (of a translation set)
 * @param $submission object  The submission to be saved.
 */
function _pxltrade_manage_submission_related_offer($master, &$submission) {
  // check if webform has related node type set (-> offer forms)
  $rel_node_type = (!empty($master->field_content_type) && $master->field_content_type[LANGUAGE_NONE][0]['value']) ?
    $master->field_content_type[LANGUAGE_NONE][0]['value'] : false;
  $is_offer_form = $rel_node_type && node_type_load($rel_node_type);

  // check if webform has a delivery type set (-> delivery forms)
  $is_delivery_form = !empty($master->field_delivery_type) && $master->field_delivery_type[LANGUAGE_NONE][0]['tid'];

  // Only process delivery and offer webforms
  if (!$is_delivery_form && !$is_offer_form) return;

  //
  // Process offer node according to the webform type (offer or delivery form)
  $offer_nid = pxltrade_webform_submission_value($master, 'offer_nid', $submission);
  if ($is_offer_form) {
    //
    // OFFER FORM SUBMISSION COMPLETION:
    // create or update submission related node with submitted values
    $status = (!empty($master->field_publish_immediately) && $master->field_publish_immediately[LANGUAGE_NONE][0]['value'] == 1) ? 1 : 0;

    if ($offer_nid) {
      // update existing offer
      $offer = node_load($offer_nid);
      $offer->status = $status;

    } else {
      // create node and save it in the submission
      $offer = new stdClass();
      $offer->type = $rel_node_type;
      $offer->language = LANGUAGE_NONE;
      $offer->uid = 1;
      $offer->status = $status;
      $offer->comment = 0;
      $offer->promote = 0;
      node_object_prepare($offer);

    }
    //
    // update submission data on the offer and save it
    _pxltrade_update_offer_with_submission_data($submission, $offer);
    $offer = node_submit($offer);
    node_save($offer);

    //
    // relate offer with submission
    foreach ($master->webform['components'] as $key => $data) {
      if ($data['form_key'] == 'offer_nid') {
        $submission->data[$key][0] = $offer->nid;
        break;
      }
    }

  }
  else if ($is_delivery_form) {
    //
    // DELIVERY FORM SUBMISSION COMPLETION:
    //
    // update the processed offer with new availability (remaining offer number and status)
    if($offer_nid) {
      $node = node_load($offer_nid);

      // check if node is still available (prevent conflict of two simultaneous customers)
      if ($node->status) {
        // offer is still available, so take it
        $remaining_offers = --$node->field_number_offer[LANGUAGE_NONE][0]['value'];
        $node->field_number_offer[LANGUAGE_NONE][0]['value'] = $remaining_offers;
        $node->status = ($remaining_offers == 0) ? 0 : 1;
        $node = node_submit($node);
        node_save($node);
      }
      else {
        // set just taken flag
        $session_data = &pxltrade_session_data();
        $session_data['just_taken'] = true;

        //
        // set 'just taken' also in submission (due to resending mails)
        foreach ($master->webform['components'] as $key => $data) {
          if ($data['form_key'] == 'offer_state') {
            $submission->data[$key][0] = 'just taken';
            break;
          }
        }
      }
    }
  }

}

/**
 * Update offer with corresponding submission values and save it.
 *
 * @param $submission object      The webform submission.
 * @param $offer       object     The offer to be updated.
 */
function _pxltrade_update_offer_with_submission_data($submission, &$offer) {
  // fill the node with the values from the submission and save it to the database
  $offer->title = $submission->data[15][0]; // title
  $offer->body[$offer->language][0]['value'] = $submission->data[3][0]; // description
  foreach ($submission->data[4] as $index => $category) {
    $offer->field_category[$offer->language][$index]['tid'] = $category; // categories
  }
  $offer->field_sex[$offer->language][0]['tid'] =  $submission->data[5][0]; // Sex
  $images = (isset($submission->data[2])) ? $submission->data[2] : array();
  foreach ($images as $index => $fid) {
    $offer->field_images[$offer->language][$index]['fid'] = $fid; // images
  }
  $offer->field_number_offer[$offer->language][0]['value'] = $submission->data[17][0]; // number of offers

  // address
  $offer->field_address[$offer->language][0]['country'] = $submission->data[18][0];  // country
  $offer->field_address[$offer->language][0]['first_name'] = $submission->data[7][0]; // first name
  $offer->field_address[$offer->language][0]['last_name'] = $submission->data[16][0]; // last name
  $offer->field_address[$offer->language][0]['thoroughfare'] = $submission->data[8][0]; // street / nr
  $offer->field_address[$offer->language][0]['postal_code'] = $submission->data[9][0]; // PLZ
  $offer->field_address[$offer->language][0]['locality'] = $submission->data[10][0]; // city
  $offer->field_subtitle[$offer->language][0]['value'] = $submission->data[10][0]; // city as subtitle

  $offer->field_phone[$offer->language][0]['value'] = (!empty($submission->data[11])) ? $submission->data[11][0] : ''; // phone
  $offer->field_email[$offer->language][0]['email'] = $submission->data[12][0]; // email
  $offer->field_delivery_form[$offer->language][0]['target_id'] = _pxltrade_convert_delivery_term_to_form($submission->data[14][0]); // delivery type/form
  if ($term = taxonomy_term_load($submission->data[14][0])) {
    $offer->field_delivery_text[$offer->language][0]['value'] = $term->name;
  }
}

function _pxltrade_convert_delivery_term_to_form($tid) {
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'webform')
    ->propertyCondition('status', 1)
    ->fieldCondition('field_delivery_type', 'tid', $tid)
    ->range(0, 1);
  $result = $query->execute();

  if ($result && !empty($result['node'])) {
    return key($result['node']);
  }
  // return default delivery form node-id (pick-up delivery)
  return 15;
}


/* ---------------------------------------------------------------------
 * Webform submission access by autonomous user (offer actions, access rights)
 * ---------------------------------------------------------------------*/
/**
 * Implements hook_webform_webform_submission_actions().
 *
 * Adds actions to edit or delete the corresponding offer of the given submission
 * by an anonymous user (token enabled access).
 */
function pxltrade_webform_submission_actions($node, $submission) {
  $actions = array();
  $token_access = $submission && isset($_GET['token']) && $_GET['token'] == webform_get_submission_access_token($submission);

  if ($token_access) {
    // make internal offer actions (secured)
    $actions['view'] = array(
      'title' => t('View offer'),
      'href' => 'pxltrade-form/' . $node->nid . '/submission/' . $submission->sid . '/anonym_view',
      'query' => drupal_get_query_parameters(),
    );
    $actions['edit'] = array(
      'title' => t('Edit offer'),
      'href' => 'pxltrade-form/' . $node->nid . '/submission/' . $submission->sid . '/anonym_edit',
      'query' => drupal_get_query_parameters(),
    );
    $actions['remove'] = array(
      'title' => t('Remove offer'),
      'href' => 'pxltrade-form/' . $node->nid . '/submission/' . $submission->sid . '/anonym_remove',
      'query' => drupal_get_query_parameters(),
    );
  }

  return $actions;
}

/**
 * Confirm form to delete a single form submission.
 *
 * @param $form
 *   The new form array.
 * @param $form_state
 *   The current form state.
 * @param $master
 *   The master webform of a translation set.
 * @param $submission
 *   The submission to be deleted (including the attached offer).
 */
function pxltrade_webform_submission_remove_form($form, $form_state, $master, $submission) {
  // Keep the NID and SID in the same location as the webform_client_form().
  // This helps mollom identify the same fields when deleting a submission.
  $form['#tree'] = TRUE;
  $form['details']['nid'] = array(
    '#type' => 'value',
    '#value' => $master->nid,
  );
  $form['details']['sid'] = array(
    '#type' => 'value',
    '#value' => $submission->sid,
  );

  $question = t('Are you sure you want to remove this offer?');
  $query = drupal_http_build_query(array('token' => $_GET['token']));
  $cancel_redirect = "pxltrade-form/{$master->nid}/submission/{$submission->sid}/anonym_view" . '?' . $query;

  return confirm_form($form, NULL, '/', $question, t('Remove'), t('Cancel'));
}

function pxltrade_webform_submission_remove_form_submit($form, &$form_state) {
  $master = node_load($form_state['values']['details']['nid']);
  $submission = webform_get_submission($form_state['values']['details']['nid'], $form_state['values']['details']['sid']);
  $offer_nid = pxltrade_webform_submission_value($master, 'offer_nid', $submission);

  // unpublish offer
  $offer = node_load($offer_nid);
  $offer->status = 0;
  node_save($offer);

  drupal_set_message(t('Offer successfully removed.'));

  // redirect to home
  $form_state['redirect'] = '/';
}

