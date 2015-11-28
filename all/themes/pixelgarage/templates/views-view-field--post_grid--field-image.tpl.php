<?php
/**
 * @file
 * This template is used to print a single field in a view.
 *
 * It is not actually used in default Views, as this is registered as a theme
 * function which has better performance. For single overrides, the template is
 * perfectly okay.
 *
 * Variables available:
 * - $view: The view object
 * - $field: The field handler object that can process the input
 * - $row: The raw SQL result that can be used
 * - $output: The processed output that will normally be used.
 *
 * When fetching output from the $row, this construct should be used:
 * $data = $row->{$field->field_alias}
 *
 * The above will guarantee that you'll always get the correct data,
 * regardless of any changes in the aliasing that might happen if
 * the view is modified.
 */

  // replace images with svg-files in postings grid
  $term = $row->_field_data['tid']['entity'];
  $name = $term->name;
  $alt = !empty($term->field_image) ? $term->field_image[LANGUAGE_NONE][0]['alt'] : '';
  $filename = !empty($term->field_image) ? $term->field_image[LANGUAGE_NONE][0]['filename'] : 'other.svg';
  $filename = str_replace(array('jpg', 'png'), 'svg', $filename);
  $path = drupal_get_path('theme', 'pixelgarage') . '/images/' . strtolower($filename);
  if (!file_exists($path)) {
    $path = drupal_get_path('theme', 'pixelgarage') . '/images/other.svg';
  }
  $url= file_create_url($path);
  $output = '<img typeof="foaf:Image" class="img-responsive" src="' . $url . '" alt="' . $alt .'" title="' . $name . '" />';
?>
<?php print $output; ?>
