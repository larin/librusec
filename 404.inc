<?
function lib404() {
  $host = $_SERVER['HTTP_HOST'];
  $ask = $_SERVER['REQUEST_URI'];
  if (preg_match('|^/index.php.q=(.+)|', $ask, $m)) $ask = $m[1];
  $u = parse_url($ask);
  $p = $u['path'];
  $q = $u['query'];
  if (preg_match('/(\d+)\.html$/', $p, $m)) return drupal_goto("a/$m[1]");  
  if (preg_match('/_ib.rus.ec.(.*)/',$p, $m)) return drupal_goto($m[1]); 
  drupal_set_title("Страница '$ask' не обнаружена");
  return "Ссылка, по которой вы пришли, не существует. Видимо, переименовалась. Это бывает.
  Если, на ваш взгляд, ссылка правильная и не сработала по ошибке - сообщите об этом, пожалуйста, администрации сайта.
<br><br>
Если вы ищете книжку, воспользуйтесь поиском книг <a href=http://$host/b>http://$host/b</a> или авторов <a href=http://$host/a>http://$host/a</a>
<br><br>
Если вы ищете что-то другое, то, вероятно, вы ошиблись сайтом. Впрочем, может вам и попадется что интересное.  Начинайте с главной страницы - <a href=http://$host/>http://$host/</a>. Удачи.";
}
