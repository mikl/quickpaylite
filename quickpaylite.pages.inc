<?php
// $Id$

/**
 * @file
 * Builds the module's user-facing pages.
 */

/**
 * The payment start-page.
 *
 * A simple page with a little info and a button that starts the payment process.
 *
 * @param $payment
 *    The loaded payment object that we're trying to process
 */
function quickpaylite_page_start($payment) {
  $output = 'Click the button to pay.';

  $output .= drupal_get_form('quickpaylite_page_payment_form', $payment);

  return $output;
}

/**
 * Build the main payment initiation form
 */
function quickpaylite_page_payment_form(&$form_state, $payment) {
  if (!is_object($payment)) {
    return drupal_not_found();
  }
  $form = array();
  $form['#method'] = 'post';
  $form['#action'] = 'https://secure.quickpay.dk/quickpay.php';
  $form['#cache'] = FALSE;

  $form['language'] = array(
    '#type' => 'hidden',
    '#value' => 'da',
  );

  $form['autocapture'] = array(
    '#type' => 'hidden',
    '#value' => '0',
  );

  $form['ordernum'] = array(
    '#type' => 'hidden',
    // Pad the ID with zeroes, since Quickpay requires four digits.
    '#value' => sprintf("%'04d", $payment->payment_id),
  );

  $form['amount'] = array(
    '#type' => 'hidden',
    '#value' => round($payment->amount * 100),
  );

  $form['currency'] = array(
    '#type' => 'hidden',
    '#value' => DKK,
  );

  $form['merchant'] = array(
    '#type' => 'hidden',
    '#value' => variable_get('quickpaylite_merchant', 0),
  );

  $form['okpage'] = array(
    '#type' => 'hidden',
    '#value' => url('payment/'. $payment->hash .'/success', array('absolute' => TRUE)),
  );

  $form['errorpage'] = array(
    '#type' => 'hidden',
    '#value' => url('payment/'. $payment->hash .'/error', array('absolute' => TRUE)),
  );

  $form['resultpage'] = array(
    '#type' => 'hidden',
    '#value' => url('payment/'. $payment->hash .'/result', array('absolute' => TRUE)),
  );

  $form['md5checkV2'] = array(
    '#type' => 'hidden',
    '#value' => md5($form['language']['#value'] . $form['autocapture']['#value'].
                    $form['ordernum']['#value'] . $form['amount']['#value'] .
                    $form['currency']['#value'] . $form['merchant']['#value'] .
                    $form['okpage']['#value'] . $form['errorpage']['#value'] .
                    $form['resultpage']['#value'] . variable_get('quickpaylite_md5secret', 0)),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Make payment'),
  );

  return $form;
}

/**
 * Success page
 */
function quickpaylite_page_success($payment) {
  return 'Yay!';
}

/**
 * Error page
 */
function quickpaylite_page_error($payment) {
  return 'Nay.';
}


/**
 * Result page.
 *
 * Captures data from Quickpay.
 */
function quickpaylite_page_result($payment) {
  // The stuff we get posted from Quickpay.
  $fields = array('amount', 'time', 'ordernum', 'pbsstat', 'qpstat', 'qpstatmsg',
                  'merchantemail', 'merchant', 'currency', 'cardtype',
                  'transaction', 'md5checkV2');

  $check = array();

  foreach ($fields as $field) {
    $entry[$field] = $_POST[$field];

    if ($field != 'md5checkV2') {
      $check[$field] = $_POST[$field];
    }
  }

  $entry['payment_id'] = $payment->payment_id;
  $entry['ip_address'] = $_SERVER['REMOTE_ADDR'];

  $md5 = md5(implode('', $check) . variable_get('quickpaylite_md5secret', 0));

  watchdog('quickpaylite', 'Entry: '. var_export($entry, TRUE));
  watchdog('quickpaylite', 'Check: '. var_export($check, TRUE));
  watchdog('quickpaylite', 'md5: '. $md5);

  if ($_POST['md5checkV2'] == $md5) {
    quickpaylite_log_write($entry);
  }

  exit('Bye.');
}

/**
 * PBS unavailable page.
 *
 * Displayed instead of payment form when we for some reason cannot clear payments
 * through PBS.
 */
function quickpaylite_page_pbs_unavailable() {
  return '<p>'. t('Sorry, we are not currently able to process your credit card with PBS. We will send you an email with instructions on how to complete your payment, when processing becomes available again.') .'</p><p>'. t('Our apologies for the inconvienience.') .'</p>';
}

/**
 * Theme a single record.
 *
 * @ingroup themeable
 */
function theme_scaffolding_example_record($record) {
  $output  = '<div class="scaffolding-example-record" id="scaffolding-example-record-' . check_plain($record['record_id']) . '">';
  $output .= '<h3 class="title">' . check_plain($record['title']) . '</h3>';
  $output .= '<p>' . check_markup($record['content']) . '</p>';
  $output .= '</div>';

  return $output;
}
