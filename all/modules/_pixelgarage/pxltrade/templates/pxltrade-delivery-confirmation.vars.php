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
  // Submission successful, complete order
  // Get the original node of the translation set (see tnid)
  $webform = $vars['node'];
  $tnid = $webform->tnid ? $webform->tnid : $webform->nid;
  $orig_form = node_load($tnid);

  $submissions = webform_get_submissions(array('nid' => $tnid, 'sid' => $vars['sid']));
  $submission = $submissions[$vars['sid']];

  // check if webform has related node type set (-> offer forms)
  $rel_node_type = (!empty($orig_form->field_content_type) && $orig_form->field_content_type[LANGUAGE_NONE][0]['value']) ?
    $orig_form->field_content_type[LANGUAGE_NONE][0]['value'] : false;
  $is_offer_form = $rel_node_type && node_type_load($rel_node_type);

  $is_delivery_form = (!empty($orig_form->field_delivery_type) && $orig_form->field_delivery_type[LANGUAGE_NONE][0]['tid']) ? true : false;

  //
  // Process webform according to its type (offer, delivery)
  $node = null;
  if ($is_offer_form) {
    //
    // OFFER FORM COMPLETION:
    // create a node of the given type and copy submission values to it
    $status = (!empty($orig_form->field_publish_immediately) && $orig_form->field_publish_immediately[LANGUAGE_NONE][0]['value'] == 1) ? 1 : 0;

    // create node
    $node = new stdClass();
    $node->type = $rel_node_type;
    $node->language = LANGUAGE_NONE;
    $node->uid = 1;
    $node->status = $status;
    $node->comment = 0;
    $node->promote = 0;
    node_object_prepare($node);

    // fill corresponding fields
    _pxltrade_create_offer_from_submission($submission, $node);

    // set the confirmation message configured in the form
    // TODO: check in case of multi-language environment
    if ($webform->webform['confirmation']) {
      $vars['confirmation_message'] = $webform->webform['confirmation'];
    }
    else {
      $vars['confirmation_message'] =
        t('<strong>Thank you very much!</strong><br> Your offer has been successfully published. Shortly you get an email with further details.');
    }


  }
  else if ($is_delivery_form) {
    //
    // DELIVERY FORM COMPLETION:
    //
    // update the processed offer
    $offer_nid = _webform_submission_value($orig_form, 'offer_nid', $submission);
    if($offer_nid) {
      $node = node_load($offer_nid);

      // check if node is still available (prevent conflict of two simultaneous customers)
      if ($node->status) {
        // offer is still available
        $remaining_offers = --$node->field_number_offer[LANGUAGE_NONE][0]['value'];
        $node->field_number_offer[LANGUAGE_NONE][0]['value'] = $remaining_offers;
        $node->status = ($remaining_offers > 0) ? 1 : 0;
        $node = node_submit($node);
        node_save($node);
        // set the confirmation message configured in the form
        // TODO: check in case of multi-language environment
        $vars['confirmation_message'] = $webform->webform['confirmation'];
      }
      else {
        // Offer not available anymore: last available offer has been taken by another customer just now
        $vars['status'] = 'just-taken';
        $vars['confirmation_message'] = t('<strong>We are very sorry!</strong><br> The offer has just been taken by another user.');
      }
    }

    //
    // display supplier contact info's, if customer could not be contacted via WhatsApp
    $session_data = &pxltrade_session_data();
    if (isset($session_data['no_contact'])) {
      $vars['status'] = 'no-contact';
      $vars['confirmation_message'] = $session_data['no_contact'];
      unset($session_data['no_contact']);
    }
  }

  // save node
  if ($node) {
    $node = node_submit($node);
    node_save($node);
  }

}


/**
 * Fill offer with corresponding submission values and save it.
 *
 * @param $submission object
 *  The webform submission.
 * @param $node object
 *  The created and prepared node with the corresponding type.
 */
function _pxltrade_create_offer_from_submission($submission, &$offer) {
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


