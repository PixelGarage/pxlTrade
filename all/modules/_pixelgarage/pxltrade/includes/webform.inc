<?php
/**
 * Creates pre-built select option lists.
 *
 * Date: 10.11.15
 * Time: 12:09
 */

/* --------------------------------------------------
 * Webform helpers
 * --------------------------------------------------*/
/**
 * Gets the master webform of a webform. Used for translated webforms to access all webform related data,
 * e.g. submissions, components etc. which are only available on the master.
 *
 * @param $node   object    The webform (translated)
 * @return bool|mixed       Returns the master webform or false, if not found.
 */
function pxltrade_webform_master_form($node) {
  $tnid = $node->tnid ? $node->tnid : $node->nid;
  return node_load($tnid);
}



/* --------------------------------------------------
 * Define pre-built select option lists for webforms
 * --------------------------------------------------*/
/**
 * Hook defining pre-built select option lists.
 *
 * @return array
 *    Array of pre-built select option definitions.
 */
function pxltrade_webform_select_options_info() {
  $items = array();

  $items['category_top'] = array(
    'title' => t('Top Categories'),
    'options callback' => 'pxltrade_webform_select_grouped_terms',
    'options arguments' => array (
      'entity_type' => 'taxonomy_term',
      'bundle' => 'category',
    ),
    'file' => 'includes/webform.inc',
  );
  //_pxltrade_webform_create_select_options_info_list('category', $items);

  $items['sex'] = array(
    'title' => t('Sex'),
    'options callback' => 'pxltrade_webform_select_option_entities',
    'options arguments' => array (
      'entity_type' => 'taxonomy_term',
      'bundle' => 'sex',
      'sort_field' => null,
      'sort_dir' => 'ASC',
    ),
    'file' => 'includes/webform.inc',
  );
  $items['delivery'] = array(
    'title' => t('Delivery type'),
    'options callback' => 'pxltrade_webform_select_option_entities',
    'options arguments' => array (
      'entity_type' => 'taxonomy_term',
      'bundle' => 'delivery_type',
      'sort_field' => null,
      'sort_dir' => 'ASC',
    ),
    'file' => 'includes/webform.inc',
  );

  return $items;
}

/**
 * Creates a select option list of all available entities of a certain type.
 * Supported entity types are: node, taxonomy_term and user
 *
 * @param $component
 *   The Webform component array for the select component being displayed.
 * @param $flat
 *   Boolean value indicating whether the returned list needs to be a flat array
 *   of key => value pairs. Select components support up to one level of
 *   nesting, but when results are displayed, the list needs to be returned
 *   without the nesting.
 * @param $arguments
 *   The "options arguments" specified in hook_webform_select_options_info().
 *
 * @return array|null An array of key => value pairs suitable for a select list's #options
 *                    FormAPI property.
 */
function pxltrade_webform_select_option_entities ($component, $flat, $arguments) {
  // collect all entity item titles and create select option list
  $entity_type = $arguments['entity_type'];
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', $entity_type)
    ->entityCondition('bundle', $arguments['bundle']);
  if ($entity_type == 'node' || $entity_type == 'user') {
    $query->propertyCondition('status', 1);
  }
  if (isset($arguments['sort_field']) && !empty($arguments['sort_field'])) {
    $query->fieldOrderBy($arguments['sort_field'], 'value', $arguments['sort_dir']);
  }
  $result = $query->execute();

  if (isset($result[$entity_type])) {
    $entity_ids = array_keys($result[$entity_type]);
    $list = array();

    switch ($entity_type) {
      case 'node':
        $nodes = node_load_multiple($entity_ids);
        foreach ($nodes as $node) {
          $list[$node->nid] = $node->title;
        }
        break;
      case 'taxonomy_term':
        $terms = taxonomy_term_load_multiple($entity_ids);
        foreach ($terms as $term) {
          $list[$term->tid] = $term->name;
        }
        break;
      case 'user':
        $users = user_load_multiple($entity_ids);
        foreach ($users as $user) {
          $list[$user->uid] = $user->name;
        }
        break;
    }

    return $list;
  }
  return null;
}

/* --------------------------------------------------
 * Create hierarchical select option list with groups
 * --------------------------------------------------*/
/**
 * Creates a grouped list of terms of a hierarchical taxonomy vocabulary.
 *
 * @param $component
 * @param $flat
 * @param $arguments
 * @return array  The grouped list of terms
 */
function pxltrade_webform_select_grouped_terms($component, $flat, $arguments){
  $list = array();
  $vocabulary = taxonomy_vocabulary_machine_name_load($arguments['bundle']);
  $tree = taxonomy_get_tree($vocabulary->vid);

  // return top level terms
  foreach ($tree as $index => $term) {
    if ($term->depth == 0) {
      $list[$term->tid] = $term->name;
      _get_children_of_term($tree, $term->tid, $list);
    }
  }


  return $list;
}


/**
 * Helper function adding all children of a term to the list.
 */
function _get_children_of_term($tree, $parent, &$list) {
  foreach ($tree as $term) {
    foreach ($term->parents as $index => $tid) {
      if ($tid == $parent) {
        $list[$term->tid] = '--  ' . $term->name;
      }
    }
  }
}


/* --------------------------------------------------
 * Create select option list with children of a parent term
 * --------------------------------------------------*/
/**
 * Create all level-1 select options list of a hierarchical taxonomy vocabulary.
 *
 * @param $bundle   string
 *    THe name of the vocabulary.
 * @param $items
 *    Reference to the select options info items.
 */
function _pxltrade_webform_create_select_options_info_list($bundle, &$items) {
  $list = array();
  $vocabulary = taxonomy_vocabulary_machine_name_load($bundle);
  $tree = taxonomy_get_tree($vocabulary->vid);

  // return top level terms
  foreach ($tree as $index => $term) {
    if ($term->depth == 0) {
      $items['category_' . $term->tid] = array(
        'title' => t('Categories of "' . $term->name . '"'),
        'options callback' => 'pxltrade_webform_select_child_terms',
        'options arguments' => array (
          'entity_type' => 'taxonomy_term',
          'bundle' => 'category',
          'parent' => $term->tid,
        ),
        'file' => 'includes/webform.inc',
      );
    }
  }
}

/**
 * Get the child terms of a term in a hierarchical taxonomy vocabulary.
 * If the argument "parent" == 0, the top level terms are returned. If the argument
 * parent is greater than 0, the value is the tid of the parent term of whom the
 * children has to be returned.
 * If the vocabulary is flat, all terms are returned.
 *
 * @param $component
 * @param $flat
 * @param $arguments
 */
function pxltrade_webform_select_child_terms($component, $flat, $arguments){
  $list = array();
  $parent = $arguments['parent'];
  $vocabulary = taxonomy_vocabulary_machine_name_load($arguments['bundle']);
  $tree = taxonomy_get_tree($vocabulary->vid);

  if ($parent == 0) {
    // return top level terms
    foreach ($tree as $index => $term) {
      if ($term->depth == 0) {
        $list[$term->tid] = $term->name;
      }
    }

  } else {
    // return children of term 'parent'
    foreach ($tree as $term) {
      foreach ($term->parents as $index => $tid) {
        if ($tid == $parent) {
          $list[$term->tid] = $term->name;
        }
      }
    }
  }

  return $list;
}

