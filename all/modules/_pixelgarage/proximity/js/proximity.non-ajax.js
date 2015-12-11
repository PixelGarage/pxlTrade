/**
 * Handles an full area item clickif AJAX is disabled.
 *
 * Created on 11.12.15.
 */

(function ($) {
  /**
   * Guarantees full area clickable proximity items in case of disabled AJAX.
   */
  Drupal.behaviors.fullSizeClickableItems = {
    attach: function () {
      // Iterate through all proximity container instances
      $.each(Drupal.settings.proximity, function (container, settings) {

        var $container = $('#' + container),
            $items     = $container.find('.pe-item-ajax');

        $items.once('click', function () {
          $(this).on('click', function () {
            window.location = $(this).find("a:first").attr("href");
            return false;
          });
        });
      });
    }
  };

})(jQuery);


