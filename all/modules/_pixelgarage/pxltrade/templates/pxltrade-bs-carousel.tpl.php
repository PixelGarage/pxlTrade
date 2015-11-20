<div id="pxltrade-bs-carousel-<?php print $id ?>" class="<?php print $classes ?>" <?php print $attributes ?>>
  <?php if ($indicators): ?>
    <!-- Carousel indicators -->
    <ol class="carousel-indicators">
      <?php foreach ($items as $key => $item): ?>
        <li data-target="#pxltrade-bs-carousel--<?php print $id ?>" data-slide-to="<?php print $key ?>" class="<?php if ($key === 0) print 'active' ?>"></li>
      <?php endforeach ?>
    </ol>
  <?php endif ?>

  <!-- Carousel items -->
  <div class="carousel-inner">
    <?php foreach ($items as $key => $item): ?>
      <div class="item <?php if ($key === 0) print 'active' ?>">
        <?php print drupal_render($item) ?>
      </div>
    <?php endforeach ?>
  </div>

  <?php if ($navigation): ?>
    <!-- Carousel navigation -->
    <a class="carousel-control left" href="#pxltrade-bs-carousel-<?php print $id ?>" data-slide="prev">
      <span class="icon-prev"></span>
    </a>
    <a class="carousel-control right" href="#pxltrade-bs-carousel-<?php print $id ?>" data-slide="next">
      <span class="icon-next"></span>
    </a>
  <?php endif ?>
</div>
