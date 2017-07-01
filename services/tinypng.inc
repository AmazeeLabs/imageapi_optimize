<?php
/**
 * @file
 * tinypng API service integration.
 */

/**
 * Implements imageapi_optimize_TYPE_NAME_info().
 */
function imageapi_optimize_services_tinypng_info() {
  return array(
    'title' => t('TinyPNG API'),
    'description' => t('- Registration is required for using this service. Obtain your free API key <a href="@tinyPNG-registration">here</a>.', array('@tinyPNG-registration' => 'https://tinypng.com/developers')),
    'url' => 'http://tinypng.com',
    'weight' => 10,
  );
}

/**
 * TinyPNG ImageAPI Optimize form callback.
 */
function imageapi_optimize_services_tinypng_form($info) {
  $form = array();

  $settings = variable_get('imageapi_optimize_tinypng', array(
    'api_key' => '',
    'debug_mode_tinypng' => FALSE,
  ));

  $form['imageapi_optimize_tinypng'] = array(
    '#type' => 'container',
    '#tree' => TRUE,
  );

  $form['imageapi_optimize_tinypng']['intro'] = array(
    '#markup' => t('<p>TinyPNG uses smart lossy compression techniques to reduce the file size of your PNG and JPEG files. By selectively decreasing the number of colors in the image, fewer bytes are required to store the data. The effect is nearly invisible but it makes a very large difference in file size!</p><p>TinyPNG compression ONLY APPLIES TO PNG and JPEG files. If your image is uploaded in other format, TinyPNG will not process the image and the default Image Toolkit processing will apply.</p><p>If you want to process existing PNG files, remove their image styles with the command <pre>drush image-flush --all</pre> and they will be recreated using TinyPNG the 1st time they are called.</p><p>Free licenses has a limit of conversions per user and per month. Please note that each time an image is uploaded to your Drupal site, an automatic request to TinyPNG is performed for EACH ONE of the Image Styles EFFECTIVELY USED for the image. This is the desired behavior, but it may quickly drain your monthly quota if many images are uploaded and many Image Styles are used.</p>')
  );

  $form['imageapi_optimize_tinypng']['api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('TinyPNG API key'),
    '#default_value' => $settings['api_key'],
    '#size' => 33,
    '#maxlength' => 32,
    '#required' => FALSE,
    '#element_validate' => array('imageapi_optimize_tinypng_validate_api_key')
  );

  $form['imageapi_optimize_tinypng']['debug_mode_tinypng'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable debug mode'),
    '#description' => t('Show statistics (size reduction) from successful requests as <a href="@url">log messages</a>.', array('@url' => url('admin/reports/dblog'))),
    '#default_value' => $settings['debug_mode_tinypng']
  );

  return $form;
}

/**
 * Validation callback for tinyPNG API key.
 * Just check that the key entered has 32-char length.
 */
function imageapi_optimize_tinypng_validate_api_key($element, &$form_state, $form) {
  if (($form_state['values']['image_toolkit'] == 'imageapi_optimize') &&
      ($form_state['values']['imageapi_optimize_service'] == 'tinypng') &&
      (strlen($element['#value']) != 32)) {
    form_error($element, t('TinyPNG API key must have 32-char length.'));
  }
}

/**
 * tinypng ImageAPI Optimize service callback.
 */
function imageapi_optimize_services_tinypng($image, $dst) {
  // Gracefully delegate to Image Toolkit if format is not PNG or JPEG.
  if (!in_array($image->info['mime_type'], array('image/png', 'image/jpeg'))) {
    return FALSE;
  }

  $filepath = drupal_realpath($dst);
  $url = 'https://api.tinypng.com/shrink';
  $settings = variable_get('imageapi_optimize_tinypng');

  $options = array(
    'data' => file_get_contents($filepath),
    'headers' => array(
      'Authorization' => 'Basic '. base64_encode("api:" . $settings['api_key']),
    ),
    'method' => 'POST',
  );
  $response = drupal_http_request($url, $options);
  $json = drupal_json_decode($response->data);

  if ($response->code == 201) {
    // Compression was successful, retrieve output
    $result = drupal_http_request($json['output']['url']);
    if (!isset($result->error)) {
      if (!file_unmanaged_save_data($result->data, $dst, FILE_EXISTS_REPLACE)) {
        $message = t('Processed image received from TinyPNG.com could not be saved to disk on @dir. Please review the directory permissions', array('@dir' => $filepath));
        watchdog('imageapi', $message);
        return FALSE;
      }

      if ($settings['debug_mode_tinypng']) {
        $message = t('Sucessful request to TinyPNG.com for @file.<br>Initial file size = @input bytes.<br>Final file size = @output bytes.<br>Compression ratio = @ratio.', array('@file' => $dst, '@input' => $json['input']['size'], '@output' => $json['output']['size'], '@ratio' => $json['output']['ratio']));
        watchdog('imageapi', $message);
      }
      return TRUE;
    }
  }

  // Something went wrong :(
  $message = t('TinyPNG.com could not process your request for @file. Error code = @error - @msg', array('@file' => $filepath, '@error' => $json['error'], '@msg' => $json['message']));
  watchdog('imageapi', $message);
  return FALSE;
}
