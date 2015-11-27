/**
 * This file contains all Drupal behaviours of the Apia theme.
 *
 * Created by ralph on 05.01.14.
 */

(function ($) {

  /**
   * This behavior adds shadow to header on scroll.
   *
   */
  Drupal.behaviors.addHeaderShadow = {
    attach: function (context) {
      $(window).on("scroll", function () {
        if ($(window).scrollTop() > 10) {
          $("header.navbar .container").css("box-shadow", "0 4px 3px -4px gray");
        }
        else {
          $("header.navbar .container").css("box-shadow", "none");
        }
      });
    }
  };

  /**
   * Implements the checkbox state for all checkboxes.
   */
  Drupal.behaviors.checkFilters = {
    attach: function () {
      var $checkboxes = $('#edit-term-node-tid-depth-wrapper .pxl-checkbox'),
          strQuery = window.location.search;

      // select/diselect checkboxes
      $checkboxes.once('checked', function() {
        $(this).on('click', function() {
          var $checkbox = $(this),
              $input = $checkbox.find('input');

          if ($input.prop('checked')) {
            $checkbox.addClass('selected');
          } else {
            $checkbox.removeClass('selected');
          }
        });
      });
    }
  };

  /**
   * Implements the active state of the filter menus and opens or closes the filter section
   * according to the menu state.
   */
  Drupal.behaviors.activateFilterMenus = {
    attach: function() {
      var $exposedForm = $('footer .footer-exposed-form'),
          $footer = $('.footer-content'),
          $menus = $footer.find('li.menu');

      $menus.once('activated', function() {
        $(this).on('click', function() {
          var $menu = $(this),
              $menuIsActive = $menu.hasClass('active'),
              $termFilters = $exposedForm.find('#edit-term-node-tid-depth-wrapper'),
              $locationFilters = $exposedForm.find('#edit-field-address-locality-wrapper');

          // reset active menu
          $menus.removeClass('active').removeClass('selected');

          // menu specific
          if ($menu.hasClass('menu-filter')) {
            $locationFilters.hide();
            $termFilters.show();

          } else if ($menu.hasClass('menu-location')) {
            $termFilters.hide();
            $locationFilters.show();

          }

          // show / hide filter section
          if(!$menuIsActive) {
            $menu.addClass('active');
            // show filter panel
            $exposedForm.slideDown(400);

          } else {
            // activate filter menu, if at least one checkbox is selected
            if ($termFilters.find('.pxl-checkbox').hasClass('selected')) {
              $footer.find('li.menu-filter').addClass('selected');
            }
            // hide filter panel
            $exposedForm.slideUp(400);
          }
        });
      });
    }
  };

  /**
   * Allows full size clickable items.
   */
   Drupal.behaviors.fullSizeClickableItems = {
    attach: function () {
      var $clickableItems = $('.pe-container .pe-item-inner');

      $clickableItems.once('click', function () {
        $(this).on('click', function () {
          window.location = $(this).find("a:first").attr("href");
          return false;
        });
      });
    }
  };

  /**
   * Swaps images from black/white to colored on mouse hover.
   Drupal.behaviors.hoverImageSwap = {
    attach: function () {
      $('.node-project.node-teaser .field-name-field-images a img').hover(
        function () {
          // mouse enter
          src = $(this).attr('src');
          $(this).attr('src', src.replace('teaser_bw', 'teaser_normal'));
        },
        function () {
          // mouse leave
          src = $(this).attr('src');
          $(this).attr('src', src.replace('teaser_normal', 'teaser_bw'));
        }
      );
    }
  }
   */

  /**
   * Open file links in its own tab. The file field doesn't implement this behaviour right away.
   Drupal.behaviors.openDocumentsInTab = {
    attach: function () {
      $(".field-name-field-documents").find(".field-item a").attr('target', '_blank');
    }
  }
   */

})(jQuery);
