<?php
/**
 * Define custom select option lists to be accessed in a webform select field.
 *
 */

/* --------------------------------------------------
 * Preprocess webform confirmation (create/update offer)
 * --------------------------------------------------*/
/**
 * Implements template_preprocess_webform_confirmation().
 */
function pxltrade_preprocess_webform_confirmation(&$vars) {
  // if we have translated webform nodes, we have to get the submissions
  // of the original node of the translation set (see tnid)
  $webform = $vars['node'];
  $tnid = $webform->tnid ? $webform->tnid : $webform->nid;

  // get the submission of the original webform node
  $orig_form = node_load($tnid);
  $submissions = webform_get_submissions(array('nid' => $tnid, 'sid' => $vars['sid']));
  $submission = $submissions[$vars['sid']];

  // check if webform (trading form) has related node type correctly set
  $rel_node_type = (!empty($orig_form->field_content_type) && $orig_form->field_content_type[LANGUAGE_NONE][0]['value']) ?
    $orig_form->field_content_type[LANGUAGE_NONE][0]['value'] : false;
  $is_trading_form = $rel_node_type && node_type_load($rel_node_type);

  // process webform according to its type
  $node = null;
  if ($is_trading_form) {
    //
    // TRADING FORM:
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
  foreach ($submission->data[2] as $index => $fid) {
    $offer->field_images[$offer->language][$index]['fid'] = $fid; // images
  }
  foreach ($submission->data[4] as $index => $category) {
    $offer->field_category[$offer->language][$index]['tid'] = $category; // categories
  }
  $offer->field_sex[$offer->language][0]['tid'] =  $submission->data[5][0]; // Party

  // address
  $offer->field_address[$offer->language][0]['country'] = 'ch';
  $offer->field_address[$offer->language][0]['first_name'] = $submission->data[7][0]; // first name
  $offer->field_address[$offer->language][0]['last_name'] = $submission->data[16][0]; // last name
  $offer->field_address[$offer->language][0]['thoroughfare'] = $submission->data[8][0]; // street / nr
  $offer->field_address[$offer->language][0]['postal_code'] = $submission->data[9][0]; // PLZ
  $offer->field_address[$offer->language][0]['locality'] = $submission->data[10][0]; // city

  $offer->field_phone[$offer->language][0]['value'] = $submission->data[11][0]; // phone
  $offer->field_email[$offer->language][0]['email'] = $submission->data[12][0]; // email
  $offer->field_preferred_delivery[$offer->language][0]['tid'] = $submission->data[14][0]; // delivery

}
