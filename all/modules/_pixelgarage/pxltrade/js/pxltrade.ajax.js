/**
 * Created by ralph on 26.11.15.
 */

(function ($) {

  /**
   * Resets the backdrop height to guarantee, that confirmation message of trading form is seen.
   *
   * @type {{attach: Function}}
   */
  Drupal.behaviors.pxlTradeAndDeliveryForm = {
    attach: function () {
      $( document ).ajaxSuccess(function( event, xhr, settings ) {
        if ( settings.url == '/system/ajax' ) {

          // Iterate through all proximity container instances
          $.each(Drupal.settings.proximity, function (container, prox_settings) {

            var $container     = $('#' + container),
                $dialog        = $container.find('.modal'),
                $errors        = $dialog.find('.messages.error');

            // adjust backdrop height and scroll to top if error occurred (to see error messages)
            if ($errors.length > 0) {
              // errors occured, scroll to top
              $dialog.scrollTop(0);
            }

            $dialog.find('.modal-backdrop').css('height', $(window).height());
          });

        }
      });
    }
  };


})(jQuery);
