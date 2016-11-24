<?php
/**
 * @file
 * Default theme implementation to display a fee block for a donation.
 *
 * Available variables are:
 * - $container_id: the id of the fee block.
 *
 * @see template_preprocess_stripe_button_fee_block()
 *
 * @ingroup themeable
 */
?>

<div id="<?php print $container_id; ?>">
  <!-- The fee section -->
  <div class="fee-text-top"><?php print $text_top; ?></div>
  <div class="fee-radios-wrapper">
    <div class="fee-radio" data-fee-value="0.0"><label>0%</label></div>
    <div class="fee-radio" data-fee-value="0.05"><label>5%</label></div>
    <div class="fee-radio selected" data-fee-value="0.1"><label>10%</label></div>
    <div class="fee-radio" data-fee-value="0.2"><label>20%</label></div>
    <div class="fee-radio" data-fee-value="0.3"><label>30%</label></div>
  </div>
  <div class="fee-text-bottom"><?php print $text_bottom; ?></div>
  <div class="fee-answer-wrapper"><?php print $text_answer; ?></div>
  <div class="fee-text-stripe-fee"><?php print $text_stripe_fee; ?></div>
</div>
