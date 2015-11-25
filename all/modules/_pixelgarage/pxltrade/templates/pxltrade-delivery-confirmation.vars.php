<?php
/**
 * Date: 20.11.15
 * Time: 22:28
 */

function template_preprocess_pxltrade_delivery_confirmation(&$vars) {
  //
  // check if submission was successful, and return with error messages when not.
  if ($vars['success'] == 'error') {
    $vars['confirmation_message'] = t('Sorry! An error occurred during the submission process.');
    return;
  }

  //
  // SUCCESSFUL DELIVERY FORM COMPLETION:
  //
  // Get the original node of the translation set (see tnid)
  $webform = $vars['node'];
  $tnid = $webform->tnid ? $webform->tnid : $webform->nid;
  $orig_form = node_load($tnid);

  $submissions = webform_get_submissions(array('nid' => $tnid, 'sid' => $vars['sid']));
  $submission = $submissions[$vars['sid']];

  //
  // update the processed offer
  $offer_nid = _webform_submission_value('offer_nid', $orig_form, $submission);
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
      $vars['success'] = 'just-taken';
      $vars['confirmation_message'] = t('We are very sorry! The offer has just now been taken by another customer.');
    }
  }

}
