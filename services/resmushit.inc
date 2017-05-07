<?php
/**
 * @file
 * ReSmush.it service integration.
 * Dedicated to old Yahoo Smush.it
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
  if (!function_exists('json_decode')) {
    drupal_set_message(t('Required function, json_decode(), is not available.'), 'error');
    return FALSE;
  }
  $dst = drupal_realpath($dst);
  $url = 'http://api.resmush.it/ws.php';
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
  if (!isset($json->error)) {
    $result = drupal_http_request($json->dest);
    if (!isset($result->error)) {
      file_unmanaged_save_data($result->data, $dst, FILE_EXISTS_REPLACE);
      return TRUE;
    }
    else {
      watchdog(
        'imageapi_optimize',
        'Cannot optimize image. drupal_http_request() call resulted in error.
        Result:<pre>@result</pre>
        API response raw:<pre>@data</pre>',
        array(
          '@result' => print_r($result, TRUE),
        ),
        WATCHDOG_WARNING
      );
    }
  }
  else {
    watchdog(
      'imageapi_optimize',
      'Cannot optimize image. ReSmush.it API call resulted in error.
      API response parsed (print_r):<pre>@json</pre>
      API response raw (print_r):<pre>@data</pre>',
      array(
        '@json' => print_r($json, TRUE),
        '@data' => print_r($data, TRUE),
      ),
      WATCHDOG_WARNING
    );
  }
}
