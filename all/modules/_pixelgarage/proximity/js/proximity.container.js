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
            $modal        = $container.find('.modal'),
            transDuration = parseInt(settings.trans_duration);

        var _modalScrolling = function() {
          var hBody,
              $modalBody    = $modal.find('.modal-body'),
              $dialog       = $modal.find('> .modal-dialog'),
              hWindow       = $(window).height(),
              hBackdrop     = Math.max(hWindow, $dialog.height());

          // add modal fixed height class and calculate body height
          if ($(window).width() < 640) {
            hBody = hWindow - $modal.find('.modal-header').height() - $modal.find('.modal-footer').height();
            $modal.addClass('modal-fixed-height');
            $modalBody.css('height', hBody);

          } else {
            $modal.removeClass('modal-fixed-height');
            $modalBody.css('height', 'auto');

          }

          // adjust backdrop height
          $modal.find('.modal-backdrop').css('height', hBackdrop);
        };


        //
        // open modal dialog, if a full item page request occurred (no ajax)
        if (settings.show_modal) {
          $modal.fadeIn(transDuration).modal('show');
          // reset flag
          settings.show_modal = false;

        }

        //
        // set modal dialog scrolling behavior when modal is opened and make sure,
        // all media is stopped on modal closing
        $modal.once('modal-hidden', function () {
          $(this).on('shown.bs.modal', function() {
            // set modal scrolling mode
            _modalScrolling();

            // disable body scrolling
            $('body').css('overflow', 'hidden');
          });

          $(this).on('hidden.bs.modal', function () {
            // empty the modal body stopping all media etc.
            $(this).find('.modal-body').empty();

            // enable background scrolling
            $('body').css('overflow', 'auto');

            // redirect to home page to update view
            window.location = '/';
          });
        });

        //
        // Modal scrolling: for windows smaller than 480px fix modal height (100%) and scroll modal body,
        // above this width scroll modal as a whole
        $(window).off('.modal-resize');
        $(window).on('resize.modal-resize', function(){
          if ($modal.is(':visible')) {
            _modalScrolling();
          }
        });

        // trigger resize for initialisation
        $(window).trigger('resize');

      }); // proximity container instances
    }
  };

})(jQuery);
