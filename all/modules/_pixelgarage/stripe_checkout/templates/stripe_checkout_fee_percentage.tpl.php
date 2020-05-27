<?php
/**
 * @file
 * Default theme implementation to display a Stripe fee percentages field.
 *
 * Available variables are:
 * - field_id:              field id
 * - fee_items:             all fee percentages to be displayed as radio buttons
 * - default_button_index:  default index of radio button to be selected
 * - top_text:              text displayed above radio buttons
 * - bottom_text:           text displayed below radio buttons
 * - stripe_fee_text:       text displayed to declare the Stripe fees
 *
 * @see template_preprocess_stripe_button_fee_percentage()
 *
 * @ingroup themeable
 */
?>

<div id="<?php print $field_id; ?>" class="stripe-button-fee-percentages">
    <!-- The fee section -->
    <div class="fee-text-top"><?php print $top_text; ?></div>
    <div class="fee-radios-wrapper">
      <?php foreach ($fee_items as $index => $fee): ?>
          <div class="fee-radio fee-radio-<?php print $index; ?> <?php if ($default_button_index == $index) print 'selected' ?>"
               data-fee-value="<?php print $fee; ?>">
              <label><?php print ($fee * 100) ?>%</label>
          </div>
      <?php endforeach ?>
    </div>
    <div class="fee-text-bottom"><?php print $bottom_text; ?></div>
    <div class="fee-answer-wrapper"><?php print $answer_text; ?></div>
    <div class="fee-text-stripe-fee"><?php print $stripe_fee_text; ?></div>
</div>
