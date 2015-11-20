<?php
/**
 * Date: 20.11.15
 * Time: 22:28
 */

function template_preprocess_pxltrade_delivery_confirmation(&$vars) {
  // if we have translated webform nodes, we have to get the submissions
  // of the original node of the translation set (see tnid)
  $webform = $vars['node'];
  $tnid = $webform->tnid ? $webform->tnid : $webform->nid;
  $orig_form = node_load($tnid);

  $submissions = webform_get_submissions(array('nid' => $tnid, 'sid' => $vars['sid']));
  $submission = $submissions[$vars['sid']];

  // set the confirmation message configured in the form
  // TODO: check in case of trnslations
  $vars['confirmation_message'] = $webform->webform['confirmation'];

  //
  // DELIVERY FORM COMPLETION:
  // disable the processed offer
  $offer_nid = null;
  foreach ($orig_form->webform['components'] as $key => $data) {
    if ($data['form_key'] == 'offer_nid') {
      $offer_nid = $submission->data[$key][0];
      break;
    }
  }
  if($offer_nid) {
    $node = node_load($offer_nid);
    $node->status = 0;
    $node = node_submit($node);
    node_save($node);
  }

}
