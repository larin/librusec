<?php

include_once drupal_get_path('module', 'librusec').'/librusec.inc';

Global $validate_errors;
// путь к каталогу со схемой
$validate_errors->xsd_path = "schema/";

function libxml_display_error($error) {
  Global $validate_errors;
  $ret = "";
  switch ($error->level) {
  case LIBXML_ERR_WARNING:
    $ret .= "<b>Warning $error->code</b>: ";
    break;
  case LIBXML_ERR_ERROR:
    $ret .= "<b>Error $error->code</b>: ";
    break;
  case LIBXML_ERR_FATAL:
    $ret .= "<b>Fatal Error $error->code</b>: ";
    break;
  }
  $ret .= trim($error->message);
  if ($error->file) $ret .= " in <b>$error->file</b>";
  $ret .= " on line <b>$error->line</b>";
  $validate_errors->s .= $ret."<br>\n";
  $validate_errors->n++;
}

function libxml_display_errors($errors) {
  foreach ($errors as $err) libxml_display_error($err);
  libxml_clear_errors();
}

function xml_validate($filename) {
  Global $validate_errors;
  libxml_use_internal_errors(true);
  $dom = new DOMDocument();
  $dom->load($filename);
  $err = libxml_get_errors();
  if (!empty($err)) {
    $validate_errors->s .= "<b>Вы уверены что это FB2?</b><br />\n";
    $validate_errors->fatal++;
    libxml_display_errors($err);
  }
  if (!$dom->schemaValidate($validate_errors->xsd_path."FictionBook2.21.xsd")) {
    $err = libxml_get_errors();
    $validate_errors->s .= "<b>Найдены ошибки!</b><br />\n";
    libxml_display_errors($err);
  }
  // возвращаем dom даже если не соответствует схеме
  return $dom;
}

function extra_validate ($dom) {
  Global $validate_errors;
  $xpath = new DOMXpath($dom);
  $xpath->registerNamespace("xlink", "http://www.w3.org/1999/xlink");
  $xpath->registerNamespace("l", "http://www.w3.org/1999/xlink");
  $href_list = array();
  $hrefs = array();
  $elements = $xpath->query("//*[@xlink:href|@l:href]");
  if (!is_null($elements)) {
    foreach ($elements as $element) {
      $name = $element->nodeName;
      $type = $element->getAttribute("type");
      $link = $element->getAttribute("l:href");
      if (!$link) {
        $link = $element->getAttribute("xlink:href");
      }
      array_push($href_list, array("name"=>$name,
                                   "type"=>$type,
                                   "link"=>$link));
      array_push($hrefs, $link);
    }
  }
  $id_list = array();
  $ids = array();
  $elements = $xpath->query("//*[@id]");
  if (!is_null($elements)) {
    foreach ($elements as $element) {
      $name = $element->nodeName;
      $id = $element->getAttribute("id");
      array_push($id_list, array("name"=>$name, "id"=>$id));
      array_push($ids, $id);
    }
  }
  foreach ($href_list as $h) {
    $name = $h["name"];
    $type = $h["type"];
    $link = $h["link"];
    if (substr($link, 0, 1) != "#") {
      if ($name == "image") {
        # внешнее изображение
        $validate_errors->s .= "<b>ERROR:</b> external image: <b>$link</b>.<br />\n";
        $validate_errors->n++;
      }
      if ($type == "note") {
        # сноска на внешний источник
        $validate_errors->s .= "<b>ERROR:</b> external note: <b>$link</b>.<br />\n";
        $validate_errors->n++;
      }
      if (substr($link, 0, strlen("http:")) != "http:" &&
          substr($link, 0, strlen("https:")) != "https:" &&
          substr($link, 0, strlen("ftp:")) != "ftp:" &&
          substr($link, 0, strlen("mailto:")) != "mailto:") {
        # плохая внешняя ссылка
        $validate_errors->s .= "<b>ERROR:</b> local external link: <b>$link</b>.<br />\n";
        $validate_errors->n++;
      }
    } else {
      # href вез соответствующего id
      $ln = substr($link, 1);
      if (!in_array($ln, $ids)) {
        $validate_errors->s .= "<b>ERROR:</b> bad internal link: <b>$link</b>.<br />\n";
        $validate_errors->n++;
      }
    }
  }
  # все ли изображения прилинкованы и будут использованы
  foreach ($id_list as $tag) {
    $name = $tag["name"];
    $id = $tag["id"];
    if ($name == "binary" && !in_array("#".$id, $hrefs)) {
      $validate_errors->s .= "<b>ERROR:</b> not linked image: <b>$id</b>.<br />\n";
      $validate_errors->n++;
    }
  }
  return;
}

function validate($filename) {
  Global $validate_errors;
  $validate_errors->n = 0;
  $validate_errors->fatal = 0;
  $validate_errors->s = '';
  if (!file_exists($filename)) {print "file $filename not found"; exit;}
  $dom = xml_validate($filename);
  if ($dom) extra_validate($dom);
  if ($validate_errors->n) {
    $validate_errors->s .= '<b style="color:red">Плохой, негодный файл</b><br />';
  } else {
    $validate_errors->s .= '<b style="color:green">Файл прошёл валидацию</b><br />';
  }
  libxml_clear_errors();
  return $validate_errors;
}
