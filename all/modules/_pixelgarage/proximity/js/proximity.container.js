/**
 * Contains all functions and behaviors to handle the content containers.
 *
 * Created by ralph on 20.11.15.
 */

(function($) {
  /**
   *  Modal dialog handling.
   *
   *  Opens the modal dialog, if a full page request is performed (no ajax involved) and
   *  guarantees, that all content is cleared (audio, video stopped) on modal dialog closing.
   */
  Drupal.behaviors.modalDialogHandling = {
    attach: function () {
      // Iterate through all proximity container instances
      $.each(Drupal.settings.proximity, function (container, settings) {

        var $container    = $('#' + container),
            $dialog       = $container.find('.modal'),
            transDuration = parseInt(settings.trans_duration);


        //
        // open modal dialog, if a full item page request occurred (no ajax)
        if (settings.show_modal) {
          $dialog.fadeIn(transDuration).modal('show');
          // reset flag
          settings.show_modal = false;

        }

        // when modal dialog is closed, make sure that all content is cleared (videos, audios etc.)
        $dialog.once('modal-hidden', function () {
          $dialog.on('hidden.bs.modal', function () {
            // empty the modal body stopping all media etc.
            $(this).find('.modal-body').empty();

            // redirect to home page to update view
            window.location = '/';
          });
        });

      }); // proximity container instances
    }
  };

})(jQuery);
