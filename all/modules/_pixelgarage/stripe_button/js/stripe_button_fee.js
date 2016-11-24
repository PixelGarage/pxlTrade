/**
 * Created by ralph on 18.05.16.
 */
(function ($) {

  /**
   * Handles the fee selection boxes.
   *
   */
  Drupal.behaviors.stripeButtonFees = {
    attach: function () {
      var $fee_block = $('#stripe-button-fee-block'),
          $fee_radios = $fee_block.find('.fee-radio'),
          $selectedItem = $fee_block.find('.fee-radios-wrapper .selected');

      $fee_radios.once('click', function() {
        $(this).on('click', function() {
          var $feeAnswerWrapper = $fee_block.find('.fee-answer-wrapper'),
              feeValue = parseFloat($(this).attr('data-fee-value')),
              params = {
                selectedFee: feeValue
              };

          // toggle selection
          $selectedItem.removeClass('selected');
          $(this).addClass('selected');
          $selectedItem = $(this);

          // update selected fee on server
          $feeAnswerWrapper.load('stripe/ajax/fee', params);
        });
      });

    }
  };

})(jQuery);

