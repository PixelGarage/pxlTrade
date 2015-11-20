(function ($) {

  /**
   * Initializes the pxltrade carousels.
   */
  Drupal.behaviors.bsCarousel = {
    attach: function(context, settings) {
      var $carousels = settings.pxltrade.carousels;

      $.each($carousels, function(id, carousel) {
        try {
          $('#pxltrade-bs-carousel-' + carousel.id).carousel(carousel.attributes);
        }
        catch(err) {
          console.log(err);
        }
      });

    }
  };

})(jQuery);
