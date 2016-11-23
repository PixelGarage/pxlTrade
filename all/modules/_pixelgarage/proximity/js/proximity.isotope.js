/**
 * Contains the initialisation of all items for an Isotope layout.
 * Created by ralph on 27.01.16.
 */

(function($) {

  /**
   * Helper function: Get isotope options.
   */
  function getIsotopeOptions(settings) {
    //
    // Set isotope options
    //
    var $options            = new Object();
    $options.itemSelector   = settings.item_selector;
    $options.percentPosition= true;
    $options.isInitLayout   = true;       // set to false, if layout should be triggered manually
    $options.isResizeBound  = true;
    $options.isOriginLeft   = true;
    $options.isOriginTop    = true;
    $options.transitionDuration = settings.trans_duration + 'ms';
    $options.containerStyle = {
      position: "relative"
    };
    $options.visibleStyle       = {
      opacity: 1
    };
    $options.hiddenStyle        = {
      opacity: 0.1
    };
    // add all stamp elements to the container
    $options.stamp = settings.stamp_elements;

    // set layout mode specific options
    $options.layoutMode = settings.layout_mode;
    switch (settings.layout_mode) {
      case 'masonry':
        $options.masonry    = {
          columnWidth: settings.grid_sizer,
          gutter: settings.gutter_sizer
        };
        break;

      case 'packery':
        $options.packery    = {
          columnWidth: settings.grid_sizer,
          gutter: settings.gutter_sizer,
          rowHeight: settings.grid_sizer,
          isHorizontal: false
        };
        break;

      case 'masonryHorizontal':
        $options.masonryHorizontal = {
          rowHeight: settings.grid_sizer,
          gutter: settings.gutter_sizer
        };
        break;
      case 'fitRows':
        $options.fitRows = {
          gutter: settings.gutter_sizer
        };
        break;
      case 'fitColumns':
        $options.fitColumns = {
          gutter: settings.gutter_sizer
        };
        break;
      case 'cellsByRow':
        $options.cellsByRow = {
          columnWidth: settings.grid_sizer,
          rowHeight: settings.grid_sizer
        };
        break;
      case 'cellsByColumn':
        $options.cellsByColumn = {
          columnWidth: settings.grid_sizer,
          rowHeight: settings.grid_sizer
        };
        break;

      case 'horizontal':
        $options.horizontal = {
          verticalAlignment: 0.5
        };
        break;

      case 'vertical':
        $options.vertical   = {
          horizontalAlignment: 0.5
        };
        break;
    }

    // return options
    return $options;
  }

  /**
   *  Isotope layout initialisation.
   *
   * This behavior applies an isotope magical layout to the container and its items.
   * The behavior is supported in IE 10+ (uses transitions), Chrome, Firefox, Safari and Opera.
   */
  Drupal.behaviors.proximityIsotopeLayouts = {
    attach: function () {
      // Iterate through all proximity container instances
      $.each(Drupal.settings.proximity, function (container, settings) {
        // set selectors and variables
        var $container             = $('#' + container),
            $modal                 = $container.find('.modal'),
            $items                 = $container.find(settings.item_selector),
            uncoverEffectEnabled   = true,
            uncoveredItems         = 0,
            isScrolling            = false,
            resizeTimeout          = null,
            infiniteScrollEnabled  = false,
            hasNextPage            = true;

        // returns true if item is visible (in viewport)
        var _inViewport = function (item) {
              var elemH      = item.height(),
                  elemTop    = item.offset().top,
                  elemBottom = elemTop + elemH,
                  viewedH    = $(window).height() + window.pageYOffset,
                  // 0.0 = item is considered in the viewport as soon as it enters,
                  // 1.0 = item is considered in the viewport only when it's fully inside
                  threshold  = 0.0,
                  inViewport = ( (elemTop + elemH * threshold) <= viewedH ) && ( (elemBottom - elemH * threshold) >= window.pageYOffset );
              return inViewport;
            };

        // disable uncover events, if all items have been shown
        var _updateUncoveredStatus = function () {
              var hasUncoveredPages = infiniteScrollEnabled && hasNextPage;
              // checks if all items are uncovered and detach events, when true
              uncoveredItems++;
              if (!hasUncoveredPages && uncoveredItems >= $items.length) {
                $(window).off('.uncover');
              }
            };

        // shows all items at once
        var _uncoverAllItems = function() {
              // update items in case of infinite scrolling (items could be added)
              if (infiniteScrollEnabled) {
                $items = $container.find('.pe-item');
              }
              $items.each(function () {
                var $this = $(this);
                if (!$this.hasClass('shown')) {
                  $this.addClass('shown').removeClass('animate');
                  uncoveredItems++;
                }
              });
               _updateUncoveredStatus();
            };

        // uncover events (scroll and resize)
        var _onScroll = function () {
              // uncover items on scrolling
              if (!isScrolling) {
                isScrolling = true;
                setTimeout(function () {
                  _uncoverAllItems();
                }, 60);
              }
            },
            _onResize = function () {
              // uncover items when resizing window
              if (resizeTimeout) {
                clearTimeout(resizeTimeout); // clear timeout as long as resizing events occur
              }
              resizeTimeout = setTimeout(function () {
                _uncoverAllItems();
                resizeTimeout = null; // resize done
              }, 1000);
            };


        //
        // Apply Isotope to container, when document and images are loaded
        //
        $options = getIsotopeOptions(settings);
        $container.imagesLoaded(function () {

          // apply isotope options
          $container.isotope($options);

          // initialize items
          if (uncoverEffectEnabled) {
            $items.each(function () {
              var $this = $(this);
              if (_inViewport($this)) {
                $this.addClass('shown');
                uncoveredItems++;
              }
            });
            _updateUncoveredStatus();

          } else {
            // make all item visible at once
            _uncoverAllItems();
          }

          // start watching for uncovered items (infinite scrolling and/or new items in viewport)
          $(window).off('uncover');
          $(window).on('scroll.uncover', _onScroll);
          $(window).on('resize.uncover', _onResize);

        });

        $modal.once('isotope', function () {
          // layout isotope after modal hiding
          $(this).on('hidden.bs.modal', function () {
            $container.isotope('layout');
          });
        });

      }); // proximity container instances
    }
  };


})(jQuery);
