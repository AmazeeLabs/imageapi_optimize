<?php
<<<<<<< HEAD

/**
 * @file
 * ReSmush.it service integration.
 *
 * Dedicated to old Yahoo Smush.it.
=======
/**
 * @file
 * ReSmush.it service integration.
 * Dedicated to old Yahoo Smush.it
>>>>>>> Added patch 12 from https://www.drupal.org/node/2458453
 */

/**
 * Implements imageapi_optimize_TYPE_NAME_info().
 */
function imageapi_optimize_services_resmushit_info() {
  return array(
    'title' => t('ReSmush.it'),
    'url' => 'http://resmush.it',
  );
}

/**
 * Smush.it ImageAPI Optimize service callback.
 */
function imageapi_optimize_services_resmushit($image, $dst) {
<<<<<<< HEAD
  $dst = drupal_realpath($dst);
  $url = 'http://api.resmush.it/ws.php';
=======
  if (!function_exists('json_decode')) {
    drupal_set_message(t('Required function, json_decode(), is not available.'), 'error');
    return FALSE;
  }
  $dst = drupal_realpath($dst);
  $url = 'http://www.resmush.it/ws.php';
>>>>>>> Added patch 12 from https://www.drupal.org/node/2458453
  $ch = curl_init();
  curl_setopt($ch, CURLOPT_URL, $url);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 5);
  curl_setopt($ch, CURLOPT_POST, TRUE);
  if (!class_exists('CURLFile')) {
    $arg = array('files' => '@' . $dst);
  }
  else {
    $cfile = new CURLFile($dst);
    $arg = array(
      'files' => $cfile,
    );
  }
  curl_setopt($ch, CURLOPT_POSTFIELDS, $arg);

  $data = curl_exec($ch);
  curl_close($ch);

  $json = json_decode($data);
  // SmushIt returns an error if it cannot optimize the image. Otherwise, it
  // returns an object, with 'dest' (temporary file) and 'percent' (savings)
  // among other properties.
<<<<<<< HEAD
  if (!isset($json->error) && isset($json->dest)) {
=======
  if (!isset($json->error)) {
>>>>>>> Added patch 12 from https://www.drupal.org/node/2458453
    $result = drupal_http_request($json->dest);
    if (!isset($result->error)) {
      file_unmanaged_save_data($result->data, $dst, FILE_EXISTS_REPLACE);
      return TRUE;
    }
  }
}
