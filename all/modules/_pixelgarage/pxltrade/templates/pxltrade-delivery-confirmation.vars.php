<?php
/**
 * Date: 20.11.15
 * Time: 22:28
 */

function template_preprocess_pxltrade_delivery_confirmation(&$vars) {
  //
  // check if submission was successful, and return with error messages when not.
  if (!$vars['success']) return;

  //
  // SUCCESSFUL DELIVERY FORM COMPLETION:
  //
  // Get the original node of the translation set (see tnid)
  $webform = $vars['node'];
  $tnid = $webform->tnid ? $webform->tnid : $webform->nid;
  $orig_form = node_load($tnid);

  $submissions = webform_get_submissions(array('nid' => $tnid, 'sid' => $vars['sid']));
  $submission = $submissions[$vars['sid']];

  // set the confirmation message configured in the form
  // TODO: check in case of multi-language environment
  $vars['confirmation_message'] = $webform->webform['confirmation'];

  //
  // disable the processed offer
  $offer_nid = _webform_submission_value('offer_nid', $orig_form, $submission);
  if($offer_nid) {
    $node = node_load($offer_nid);
    $node->status = 0;
    $node = node_submit($node);
    node_save($node);
  }

}
